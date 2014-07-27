#!/usr/bin/python
from operator import itemgetter
import sys

prevColumn = '0'
currColumn = '0'
newline = {}
#input from standard input
for line in sys.stdin:
		
	# Removing whitespace to produce more readable output.
	line = line.strip()

	# We split the key-value pairs.
	#linekey,lineval = line.split('\t',1)
	partitionid, columnId, rowId, num = line.split('\t')
	currColumn = columnId
	# If current column is the same as the previous column then we are still on the same line
	# and we fill our newline dictionary with the number we received at its corresponding row index.
	# By doing so we will use the number's column index as a row index and its row as column.	
	if currColumn == prevColumn:	
		newline[int(rowId)] = num
	else:
		# Else we print the values in newline and empty the dictionary after we are done.
		for i in newline: 
			if i == (len(newline)-1):
				print newline [i]
			else :
				print newline [i],
		newline = {}	
		
		newline[int(rowId)] = num	
		 
	prevColumn = columnId
# In the end of the loop iteration we print the last line of the transposed matrix.
if currColumn == prevColumn:
	for i in newline: 
		if i == (len(newline)-1):
			print newline [i]
		else :
			print newline [i],

