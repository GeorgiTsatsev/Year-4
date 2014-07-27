#!/usr/bin/python
import sys

#input from standard input
for line in sys.stdin:
	
	# Removing whitespace to produce more readable output.
	line=line.strip()

	# Changes the current line string into a string with uppercases.
	line = line.upper()
	print '%s' % line


