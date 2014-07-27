#!/bin/bash
hadoop jar /opt/hadoop/hadoop-0.20.2/contrib/streaming/hadoop-0.20.2-streaming.jar -input /user/s1250553/ex2/uniLarge.txt -output /user/s1045049/data/output/s1045049_task_8.out/ -mapper mapper.py -file ~/Downloads/Courses/4/EXC/2/task8/mapper.py -reducer reducer.py -file ~/Downloads/Courses/4/EXC/2/task8/reducer.py

