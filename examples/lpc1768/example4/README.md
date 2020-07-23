# Example 1
This example project implements a *Rate Monotonic Scheduling* (RMS) policy and perform an online slack calculation.

Each task print the following data into the serial port: 
* Its assigned name, followed by a `S` character if the task is Starting, or an `E` if it is ending.
* The current tick count.
* The system available slack, followed by the slack counters for all the tasks.
* The current task executed time, measured in ticks.

This is a example output:
```
Example 1
T1  S   0       1000    2000    1000    1000    2000    0
T1  E   570     1000    2000    1000    1000    2000    570
T2  S   602     1398    4398    1398    1398    2398    0
T2  E   1174    1398    3826    1398    1398    2398    572
T3  S   1206    1794    3794    3794    1794    2794    0
T3  E   1778    1794    3222    3222    1794    2794    572
T4  S   1810    3190    3190    3190    4190    3190    0
T4  E   2382    2618    2618    2618    3618    3190    572
T1  S   3000    2000    2000    2000    3000    5000    0
T1  E   3570    2000    2000    2000    3000    5000    570
T2  S   4000    2000    4000    2000    3000    5000    0
T2  E   4570    2000    3430    2000    3000    5000    570
T1  S   6000    2000    2000    3001    2001    4001    0
T1  E   6570    2000    2000    3001    2001    4001    570
T3  S   6602    2399    4398    3399    2399    4399    0
T3  E   7174    2399    3826    2827    2399    4399    572
T2  S   8000    2001    3000    2001    3000    4001    0
T2  E   8570    2001    2430    2001    3000    4001    570
T1  S   9000    2000    2000    3001    3001    4002    0
T1  E   9570    2000    2000    3001    3001    4002    570
T1  S   12000   1001    2000    1001    1001    2002    0
T1  E   12570   1001    2000    1001    1001    2002    570
T2  S   12602   1399    4398    1399    1399    2400    0
T2  E   13174   1399    3826    1399    1399    2400    572
T3  S   13206   1795    3794    3794    1795    2796    0
T3  E   13778   1795    3222    3222    1795    2796    572
T4  S   13811   3189    3189    3189    4189    3191    0
T4  E   14382   2618    2618    2618    3618    3191    571
...
```

Note that the `pc.printf()` function has a considerable overhead.
