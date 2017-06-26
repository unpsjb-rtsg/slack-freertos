# Example 1
This example project implements a *Rate Monotonic Scheduling* (RMS) policy, and performs an online slack calculation.

Each task print the following data into the serial port: 
* Its assigned name, followed by a `S` character if the task is Starting, or an `E` if it is ending.
* The current tick count.
* The system available slack, followed by the slack counters for all the tasks.
* The current task executed time, measured in ticks.

This is a example output:
```
Example 1
T1  S   0       1000    2000    1000    1000    2000    0
T1  E   891     1000    2000    1000    1000    2000    891
T2  S   924     1076    4075    1076    1076    2076    0
T2  E   1817    1076    3183    1076    1076    2076    893
T3  S   1850    1150    3150    3149    1150    2150    0
T3  E   2743    1150    2257    2257    1150    2150    893
T4  S   2776    2224    2224    2224    3223    2224    0
T1  S   3000    2000    2000    2000    3000    2224    0
T1  E   3891    2000    2000    2000    3000    2224    891
T2  S   4000    2000    4000    2000    3000    2300    0
T2  E   4892    2000    3108    2000    3000    2300    892
T4  E   5519    2376    2481    3481    2482    2376    894
T1  S   6000    2000    2000    3000    2001    4000    0
T1  E   6891    2000    2000    3000    2001    4000    891
T3  S   6924    2077    4075    3076    2077    4076    0
T3  E   7817    2077    3183    2183    2077    4076    893
T2  S   8000    2000    3000    2000    3000    4000    0
T2  E   8892    2000    2108    2000    3000    4000    892
T1  S   9000    2000    2000    3000    3001    4001    0
T1  E   9891    2000    2000    3000    3001    4001    891
T1  S   12000   1000    2000    1000    1001    2001    0
T1  E   12891   1000    2000    1000    1001    2001    891
```

Note that the `pc.printf()` function has a considerable overhead.
