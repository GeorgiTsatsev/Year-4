#!/usr/bin/python
from operator import itemgetter
import sys

lineCount = 0
wordCount = 0
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()
	
	# We split the key-value pairs.
	words = line
	words = int (words)

	# Since we itterate the loop for every line the line count would simply be the number of times we have looped.	
	lineCount += 1
	
	# We got each lines word count in the mapper so just add these values for every loop itteration to get the total word count.
	wordCount = wordCount + words

print '%s %s' % (wordCount,lineCount)

