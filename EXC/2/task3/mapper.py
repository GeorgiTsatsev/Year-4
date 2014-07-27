#!/usr/bin/python
import sys

#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()

	tokens = line.split()
	# Mapper outputs the number of words within each line.
	print '%s' % len(tokens)
