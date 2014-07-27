#!/usr/bin/python
from operator import itemgetter
import sys
import numpy as np


exp = 0
wordCount = 0
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()
	
	# We split the key-value pairs.
	words = int (line)
	
	# Randomized counting for lines.
	random = np.random.random_sample()
	if exp == 0:
		exp =1
	else:
		if random <= pow(2,-exp):
				exp+=1 

	
	# We got each lines word count in the mapper so just add these values for every loop itteration to get the approximate total word count.
	wordCount = wordCount + words

print '%s %s' % (wordCount,(pow(2,exp)-1))

