#!/bin/bash
hadoop jar /opt/hadoop/hadoop-0.20.2/contrib/streaming/hadoop-0.20.2-streaming.jar -input  /user/s1045049/data/output/s1045049_task_5.out/ -output /user/s1045049/data/output/s1045049_task_6.out/ -mapper mapper.py -file ~/Downloads/Courses/4/EXC/2/task6/mapper.py -reducer reducer.py -file ~/Downloads/Courses/4/EXC/2/task6/reducer.py -jobconf mapred.reduce.tasks=1 -jobconf mapred.output.key.comparator.class=org.apache.hadoop.mapred.lib.KeyFieldBasedComparator -jobconf mapred.text.key.comparator.options=-nr

