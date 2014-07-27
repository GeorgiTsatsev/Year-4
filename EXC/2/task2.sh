#!/bin/bash
hadoop jar /opt/hadoop/hadoop-0.20.2/contrib/streaming/hadoop-0.20.2-streaming.jar -input /user/s1045049/data/output/s1045049_task1.out/* -output /user/s1045049/data/output/s1045049_task2.out/ -mapper mapper.py -file ~/Downloads/Courses/4/EXC/2/task2/mapper.py -reducer reducer.py -file ~/Downloads/Courses/4/EXC/2/task2/reducer.py 

