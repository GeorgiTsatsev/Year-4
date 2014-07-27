#!/usr/bin/python
from operator import itemgetter
import sys

sentence = ""
#input from standard input
for line in sys.stdin:
	
	line = line.strip()
	# We get the sentence(line) from the key-value pair.	
	sentence, value = line.split('\t' , 1)

	# Since we don't care about the duplicates we simple print the sentence.
	print '%s' % sentence
