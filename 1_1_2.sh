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

# Create a new file that contains the content of the words & numbers merged side by side and save it to a
# file called “MergedContent”.
# the -d', ' argument specifies to use a comma and space as a delimiter between the contents of each file
cd Lab1
paste -d', ' words numbers >> MergedContent

# Display the first three lines in the file MergedContent.
head -3 MergedContent