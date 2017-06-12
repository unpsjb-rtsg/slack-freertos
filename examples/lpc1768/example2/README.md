# Slack Stealing example 2
Example project that uses a *Rate Monotonic* scheduler with *Slack Stealing*. Each task prints the slack counters for all tasks and the system available slack. The available slack is used to schedule an aperiodic task, which is ready to run at random times.

This is a example execution output:

```
Example 2
TA  S   0   1000    2000    1000    1000    2000    0
T1  S   1001    0   1000    0   0   1000    0
T1  E   1876    0   1000    0   0   1000    875
T2  S   2001    0   2999    0   0   1000    0
T2  E   2877    0   2123    0   0   1000    876
T1  S   3001    0   1999    1999    0   1000    0
T1  E   3876    0   1999    1999    0   1000    875
T2  S   4001    0   3999    1999    0   1000    0
T2  E   4877    0   3123    1999    0   1000    876
T3  S   5001    0   2999    3999    0   1000    0
T3  E   5877    0   2123    3123    0   1000    876
TA  E   6838    163 1162    2162    1163    163 2330
T1  S   6871    130 1129    2129    1130    130 0
T1  E   7764    130 1129    2129    1130    130 893
T3  S   7797    204 3202    2203    1204    204 0
T2  S   8000    204 3000    2000    1204    204 0
TA  S   8804    204 2196    2000    1204    204 0
T1  S   9008    0   1992    1796    1000    0   0
T1  E   9883    0   1992    1796    1000    0   875
T2  E   10096   0   3904    1796    1000    0   892
T3  E   10895   0   3105    2106    1000    0   894
T4  S   11001   0   2999    2000    1999    0   0
T4  E   11877   0   2123    1124    1123    0   876
T1  S   13000   0   1000    1   0   1000    0
T1  E   13875   0   1000    1   0   1000    875
T2  S   14000   0   3000    1   0   1000    0
T2  E   14876   0   2124    1   0   1000    876
T1  S   15000   0   2000    2000    0   1000    0
T1  E   15875   0   2000    2000    0   1000    875
T2  S   16000   0   4000    2000    0   1000    0
T2  E   16876   0   3124    2000    0   1000    876
T3  S   17000   0   3000    4000    0   1000    0
T3  E   17876   0   2124    3124    0   1000    876
```
