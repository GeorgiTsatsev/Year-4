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
	columnId, rowId, num = line.split('\t',2)
	currColumn = columnId
	if currColumn == prevColumn:	
		newline[int(rowId)] = num
	else:
		for i in newline: 
			if i == 0 :
				print columnId,
			print newline [i],
		newline = {}		
		newline[int(rowId)] = num	
		 
	prevColumn = columnId
if currColumn == prevColumn:
	print columnId,
	for i in newline: 
		print newline [i],
		

# COMMAND FOR SORT
# create 
#hadoop dfs -cat /user/s1045049/data/output/* | sort -n | awk -F 'FS' 'BEGIN{FS=" "}{for (i=1; i<=NF-1; i++) if(i!=1) {printf $i FS};{print $NF}}' | hadoop dfs -put - /user/s1045049/data/output/exc7/finalResult.txt

