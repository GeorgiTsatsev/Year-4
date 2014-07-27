#!/bin/bash
hadoop jar /opt/hadoop/hadoop-0.20.2/contrib/streaming/hadoop-0.20.2-streaming.jar -input /user/s1250553/ex2/webLarge.txt -output /user/s1045049/data/output/s1045049_task_3.out/ -mapper mapper.py -file ~/Downloads/Courses/4/EXC/2/task3/mapper.py -reducer reducer.py -file ~/Downloads/Courses/4/EXC/2/task3/reducer.py

hadoop dfs -cat /user/s1045049/data/output/s1045049_task_3.out/* | awk '{column1+=$1}{column2+=$2}END{print column1,column2}' | hadoop dfs -put - /user/s1045049/data/output/s1045049_task_3.out/finalResult.txt

