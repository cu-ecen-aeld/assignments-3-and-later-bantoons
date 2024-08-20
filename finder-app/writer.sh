#!/bin/bash

# Test if the number of arguments is equal to 2
if [ $# != 2 ]; then
     echo "Incorrect Arguments"
    exit 1
fi

# Use the same arguments name as state on the Assignment 1 Instruction
writefile=$1
writestr=$2
writedir=$(dirname "$writefile")

# Create the directory path
mkdir -p "$writedir"

# Write the file and its path
echo "$writestr" > "$writefile"

#Create the file and if itc can create, write the contenr
if touch "$writefile"; then
	echo "$writestr" > "$writefile"
else
	echo "Error creating file"
fi

