#!/usr/bin/python
import sys

key = ""
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()
	tokens = line.split()
	
	print '%s\t%s\t%s\t%s' % (tokens[3],tokens[0],tokens[1],tokens[2])

