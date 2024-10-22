#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT "9000"
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define TEMP_PATH "/var/tmp/aesdsocketdata"

static volatile bool caught_signal = false;
static int sockfd = -1;
static int client_fd = -1;
static FILE* file_ptr = NULL;

//Signal Handler 
void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        caught_signal = true;
    }
}

//Clean Before Exit the Application
void clean_exit() {
    if (file_ptr != NULL) {
        fclose(file_ptr);
    }
    if (client_fd != -1) {
        close(client_fd);
    }
    if (sockfd != -1) {
        close(sockfd);
    }
    remove(TEMP_PATH);
    closelog();
}


//Socket Setup and Binding
int setup_socket() {
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    //hints.ai_family = AF_UNSPEC;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        syslog(LOG_ERR, "getaddrinfo: %s", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            syslog(LOG_ERR, "setsockopt failed");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        syslog(LOG_ERR, "Failed to bind");
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        syslog(LOG_ERR, "listen failed");
        return -1;
    }

    return 0;
}


//Client Handling when accpeted
void handle_client(int client_fd) {
    char rx_buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    size_t total_bytes = 0;

    file_ptr = fopen(TEMP_PATH, "a+");
    if (file_ptr == NULL) {
        syslog(LOG_ERR, "Error opening file: %s", strerror(errno));
        return;
    }

    while ((bytes_received = recv(client_fd, rx_buffer, sizeof(rx_buffer) - 1, 0)) > 0) {
        rx_buffer[bytes_received] = '\0';
        fputs(rx_buffer, file_ptr);
        fflush(file_ptr);
        
        total_bytes += bytes_received;

        if (strchr(rx_buffer, '\n')) {
            fseek(file_ptr, 0, SEEK_SET);
            while ((bytes_received = fread(rx_buffer, 1, sizeof(rx_buffer), file_ptr)) > 0) {
                send(client_fd, rx_buffer, bytes_received, 0);
            }
            break;
        }
    }

    fclose(file_ptr);
    file_ptr = NULL;
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    bool daemon_mode = false;

    openlog("aesdsocket", LOG_PID, LOG_USER);

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = true;
                break;
            default:
                syslog(LOG_ERR, "Usage: %s [-d]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set up signal handling
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
    // Start socket
    if (setup_socket() == -1) {
        exit(EXIT_FAILURE);
    }

    // Daemon mode
    if (daemon_mode) {
        if (daemon(0, 0) == -1) {
            syslog(LOG_ERR, "Failed to run as daemon");
            exit(EXIT_FAILURE);
        }
    }

    syslog(LOG_INFO, "aesdsocket started");

    while (!caught_signal) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "Error accepting connection");
            break;
        }

        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(client_addr.sin_family, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        handle_client(client_fd);

        close(client_fd);
        client_fd = -1;
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    clean_exit();
    return EXIT_SUCCESS;
}
