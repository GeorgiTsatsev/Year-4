#!/usr/bin/python
from operator import itemgetter
import sys

prevSent = ""
sentence = ""

#input from standard input
for line in sys.stdin:

	line = line.strip()	
	sentence = line
	
	# If the previous sentence is the same as our current one then we have a duplicate
	# and we don't print it until a different sentence is encountered.
	if prevSent != sentence:
		print '%s' % sentence
	
	# Assign value for the previous sentence for the next loop itteration.	
	prevSent=sentence
