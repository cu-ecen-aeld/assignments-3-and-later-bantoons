#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* th_data = (struct thread_data *)thread_param;
    
    // Wait before obtaining mutex
    usleep(th_data->wait_to_obtain_ms * 1000);
    
    // Obtain mutex
    if (pthread_mutex_lock(th_data->mutex) != 0) {
        ERROR_LOG("Locking mutex failed \n");
        th_data->thread_complete_success = false;
        return thread_param;
    }
    
    // Wait before releasing mutex
    usleep(th_data->wait_to_release_ms * 1000);
    
    // Release mutex
    if (pthread_mutex_unlock(th_data->mutex) != 0) {
        ERROR_LOG("ULocking mutex faile \n");
        th_data->thread_complete_success = false;
        return thread_param;
    }
    
    th_data->thread_complete_success = true;


    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    /* Memory allocation for thread data structure*/
    struct thread_data *th_data = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (!th_data) {
        ERROR_LOG("Memory allocation Failed \n");
        return false;
    }

    th_data->mutex = mutex;
    th_data->wait_to_obtain_ms = wait_to_obtain_ms;
    th_data->wait_to_release_ms = wait_to_release_ms;
    th_data->thread_complete_success = false;

    /* Start thread*/
    int result = pthread_create(thread, NULL, threadfunc, th_data);
    if (result != 0) {
        ERROR_LOG("Thread create Failed \n");
        free(th_data);
        return false;
    }

    /*Thread created succesfull*/
    return true;
}

