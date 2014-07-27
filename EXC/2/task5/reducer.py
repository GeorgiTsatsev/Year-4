#!/usr/bin/python
from operator import itemgetter
import sys

prevSeq = []
currSeq = []
countTotal = 1
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()
	
	# We split the key-value pairs.
	sequence, count = line.split('\t',1)
		
	currSeq = sequence
	
	# If we the current word sequence is equalt ot the previous we increment its count.
	if prevSeq == currSeq:
		countTotal+=1
	else :
		# Check to skip printing when prevSeq is empty.
		if prevSeq:
			print '%s\t%s' % (prevSeq,countTotal)
		countTotal = 1
		prevSeq = sequence
# print the last sequence after the loop iteration.
if prevSeq == sequence:
	print '%s\t%s' % (prevSeq,countTotal)
