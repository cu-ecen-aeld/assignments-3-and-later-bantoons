#!/bin/bash

# Test if the number of arguments is equal to 2
if [ $# != 2 ]; then
    echo "Incorrect Arguments"
    exit 1
fi

# Use the same arguments name as state on the Assignment 1 Instruction
filesdir=$1
searchstr=$2

# Test directory exists ?
if [ ! -d "$filesdir" ]; then
    echo "Error: $filesdir is mot a direstory or does not exist"
    exit  1
fi

# Count the number of files
file_count=$(find "$filesdir" -type f | wc -l)

# Count the number of matching lines
match_count=$(grep -r -o "$searchstr" "$filesdir" | wc -l)

# Print the results
echo "The number of files are $file_count and the number of matching lines are $match_count"

