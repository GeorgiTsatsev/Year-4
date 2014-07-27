#!/usr/bin/python
import sys

#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()
	i = 0
	tokens = line.split()
	
	# If we have less than three words in a line then we skip that line.
	if len(tokens)>=3:
		
		# We loop through each word in the sentence until we reach the last three word sequence.
		while i<=(len(tokens)-3):
			
			# Check used to get rid of unwanted characters ( only numbers and letters are passed to the reducers).
			if tokens[i].isalnum()&tokens[i+1].isalnum()&tokens[i+2].isalnum():
				seq = tokens[i:(i+3)]						
				print '%s\t%s' % ((' '.join(seq)), 1)
			i+=1
