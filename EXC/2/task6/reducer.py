#!/usr/bin/python
from operator import itemgetter
import sys

#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()
	tokens = line.split('\t')
	print '%s %s %s\t%s' % (tokens[1],tokens[2],tokens[3],tokens[0])
	

