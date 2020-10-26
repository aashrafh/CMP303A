#!/bin/bash

# Delete the directory Lab1 if it exists.
if [ -d Lab1 ] 
then 
	rm -r Lab1
fi

# Create a new directory called Lab1.
mkdir Lab1

# Copy the files “words” & “numbers” 
cp words numbers Lab1