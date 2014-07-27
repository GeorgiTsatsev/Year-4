#!/usr/bin/python
import sys

#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()
	tokens = line.split()
	index = 0
	for token in tokens[1:]:
		print '%s\t%s\t%s' % (index,tokens[0],token)
		index+=1
		
