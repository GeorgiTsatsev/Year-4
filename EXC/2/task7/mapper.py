#!/usr/bin/python
import sys

#input from standard input
for line in sys.stdin:
	
	# Removing whitespace to produce more readable output.
	line=line.strip()
	tokens = line.split()
	
	# Column index of input matrix.
	index = 1
	
	#  Used for determining how many key-value pairs would be mapped to the same partition.
	# We divide by 10 since we are currently using 10 reducers.
	perpart =(len(tokens)-1)/10
	
	# partitionid starts from 2 since it was not sorted properly with 0 or 1.	
	partitionid=2

	# We loop through each number in our matrix row to print its designated partition, column and row index and itself in the end.
	for token in tokens[1:]:
		if index % perpart==0 :
			partitionid+=1	

		print '%s\t%s\t%s\t%s' % (partitionid,index,tokens[0],token)
		index+=1
		
