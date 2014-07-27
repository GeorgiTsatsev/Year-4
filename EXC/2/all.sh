#!/bin/bash
hadoop dfs -rmr /user/s1045049/data/output/
rm -rf ~/Downloads/Courses/4/EXC/2/outputTotal/output
rm ~/Downloads/Courses/4/EXC/2/outputTotal/finalResult.txt

hadoop jar /opt/hadoop/hadoop-0.20.2/contrib/streaming/hadoop-0.20.2-streaming.jar -input /user/s1045049/data/input/webSmall.txt -output /user/s1045049/data/output -mapper mapper4.py -file ~/Downloads/Courses/4/EXC/2/mapper4.py -reducer reducer4.py -file ~/Downloads/Courses/4/EXC/2/reducer4.py 

hadoop dfs -copyToLocal /user/s1045049/data/output/ ~/Downloads/Courses/4/EXC/2/outputTotal/ 
hadoop dfs -getmerge /user/s1045049/data/output/ ~/Downloads/Courses/4/EXC/2/outputTotal/finalResult.txt
