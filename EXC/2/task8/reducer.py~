#!/usr/bin/python
from operator import itemgetter
import sys


var = ""
prev = 0
curr = 0

#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()
	tokens = line.split('\t')
	curr = int (tokens[0])
	if curr==prev:
		# Condition to check if we have received input from the marks or students database.
		# If its from the student we simply print the name followed by -->. Otherwise we update
		# a string filled with course, mark tuples. which we print after we encounter the name 
		# to which they correspond to.	
		if len(tokens)==2:
			name = tokens[1]
			print '%s-->' % name,
				
		else :
			course = tokens[1]
			mark = tokens[2]	
			var = var + '(%s, %s) ' % (course, mark)	
		
	else:

		# We print the string with course mark tuples and empty its content when the student id changes and check if we currently have an entry
		# from the student or the mark database. If studentname database entry we print the name
		# otherwise we update the course mark string.
		print ' %s' % (var)
		var = ""
		if len(tokens)==2:
			name = tokens[1]
			
			print '%s--> ' % name,		
		else:
			course = tokens[1]
			mark = tokens[2]
			var = var + '(%s, %s) ' % (course, mark)			
			
	prev = curr

