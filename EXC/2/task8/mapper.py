#!/usr/bin/python
import sys

key = ""
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line=line.strip()
	tokens = line.split()
	
	# Two if statements to determine which relational database have encountered.
	# depending on it we send different key-value pairs skipping the student or mark token.
	# The key in both cases is the studentId.
	if tokens[0] == "student":
		key = tokens[1]
		print '%s\t%s' % (key,tokens[2])
	if tokens[0] == "mark":
		key =tokens[2]
		print '%s\t%s' %(key,tokens[1]+ "\t" + tokens[3])

