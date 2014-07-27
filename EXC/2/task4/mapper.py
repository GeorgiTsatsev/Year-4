#!/usr/bin/python
import sys
import numpy as np


#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()
	
	# The exponent need to be set to zero at the start of each line's word counter. 	
	exp = 0
	tokens = line.split()

	# We loop through each word and use the randomised counter in each itteration.
	for token in tokens:
		
		# Uniformly distributed number between 0 and 1.		
		random = np.random.random_sample()
		if exp == 0:
			exp =1
		else:
			# Only if the random number is less then 2^-exp we increment the exponential.
			if random <= pow(2,-exp):
				exp+=1 
		
	# Printing result of the random counter for each line's word counter.
	print '%s' % (pow(2,exp)-1)

