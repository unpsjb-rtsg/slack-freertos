# Example 2
This example project implements a *Rate Monotonic Scheduling* (RMS) policy and performs an online slack calculation. The available slack is used to schedule two aperiodic tasks, which are ready to run at random times.

Each task print the following data into the serial port: 
* Its assigned name, followed by a `S` character if the task is Starting or an `E` if it is ending.
* The current tick count.
* The system available slack, followed by the slack counters for all the tasks.
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
Note that the `pc.printf()` function has a considerable overhead.

A detailed trace of the system execution is generated using the [Percepio Tracealyzer](https://percepio.com/tz/) library. By default the example uses the [Snapshot Mode](https://percepio.com/docs/FreeRTOS/manual/Recorder.html#Trace_Recorder_Library_Snapshot_Mode), which keeps the trace in a buffer, allowing for saving a snapshot at any point by making a memory dump.

For example, using CMSIS-DAP a memory dump could be done starting the `pyocd-gdbserver` with the FRDM-K64F board connected, and then executing the following commands in `arm-none-eabi-gdb` debugger session:
```bash
$ arm-none-eabi-gdb
(gdb) target remote localhost:3333
(gdb) file example2.elf
(gdb) load
(gdb) continue
^C
(gdb) dump binary memory example2.dump 0x1000FFFF 0x2002FFFF
(gdb) quit
$ 
```

The trace can be then inspected opening the `example2.dump` file with the *Tracealyzer for FreeRTOS* application.
