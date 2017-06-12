# Slack Stealing example 2
Example project that uses a *Rate Monotonic* scheduler with *Slack Stealing*. The available slack is used to schedule an aperiodic task, which is ready to run at random times.

Each task print the following data into the serial port: 
* Its name, followed by a S if the task is Starting, or an E if it is ending.
* The current tick count.
* The slack counters for all the tasks, followed by the system available slack.
* The current task executed time, measured in ticks.

This is a example output:

```
Example 2
TA  S   0       1000    2000    1000    1000    2000    0
T1  S   1001    0       1000    0       0       1000    0
T1  E   1876    0       1000    0       0       1000    875
T2  S   2001    0       2999    0       0       1000    0
T2  E   2877    0       2123    0       0       1000    876
T1  S   3001    0       1999    1999    0       1000    0
T1  E   3876    0       1999    1999    0       1000    875
T2  S   4001    0       3999    1999    0       1000    0
T2  E   4877    0       3123    1999    0       1000    876
T3  S   5001    0       2999    3999    0       1000    0
T3  E   5877    0       2123    3123    0       1000    876
TA  E   6838    163     1162    2162    1163    163     2330
T1  S   6871    130     1129    2129    1130    130     0
T1  E   7764    130     1129    2129    1130    130     893
...
```

Note that the `pc.printf()` functions has a considerable overhead.