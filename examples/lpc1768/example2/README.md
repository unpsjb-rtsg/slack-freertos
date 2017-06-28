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
T1  S   0       1000    2000    1000    1000    2000    0
T1  E   846     1000    2000    1000    1000    2000    846
T2  S   879     1121    4121    1121    1121    2121    0
T2  E   1727    1121    3273    1121    1121    2121    848
TA2 S   1759    1105    3241    1105    1105    2105    0
TA1 S   1933    931     3067    931     931     1931    0
T3  S   2984    0       2016    2016    0       1000    0
T1  S   3000    0       2000    2000    0       1000    0
T1  E   3848    0       2000    2000    0       1000    848
T2  S   4000    0       4000    2000    0       1000    0
T2  E   4831    0       3169    2000    0       1000    831
T3  E   5815    0       2185    3185    0       1000    831
TA1 E   6236    748     1764    2764    1764    748     1697
TA2 E   6413    571     1587    2587    1587    571     319
T1  S   6446    538     1554    2554    1554    538     0
T1  E   7293    538     1554    2554    1554    538     847
T3  S   7326    658     3674    2674    1674    658     0
TA1 S   7968    658     3032    2032    1674    658     0
T2  S   8626    0       2374    1374    1016    0       0
T1  S   9000    0       2000    1374    1016    0       0
T1  E   9830    0       2000    1374    1016    0       830
T2  E   10457   0       3543    1374    1016    0       831
T3  E   10832   0       3168    2168    1016    0       848
T4  S   10984   0       3016    2016    2016    0       0
T4  E   11815   0       2185    1185    1185    0       831
TA1 E   12735   265     1265    265     265     1265    1939
TA2 S   12768   232     1232    232     232     1232    0
T1  S   13000   0       1000    0       0       1000    0
T1  E   13830   0       1000    0       0       1000    830
T2  S   14000   0       3000    0       0       1000    0
T2  E   14831   0       2169    0       0       1000    831
...
```

Note that the `pc.printf()` function has a considerable overhead.

A detailed trace of the system execution is generated using the [Percepio Tracealyzer](https://percepio.com/tz/) library. By default the example uses the [Snapshot Mode](https://percepio.com/docs/FreeRTOS/manual/Recorder.html#Trace_Recorder_Library_Snapshot_Mode), which keeps the trace in a buffer, allowing for saving a snapshot at any point by making a memory dump.

For example, using CMSIS-DAP a memory dump could be done starting the `pyocd-gdbserver` with the mbed LPC1768 board connected, and then executing the following commands in `arm-none-eabi-gdb` debugger session:
```bash
$ arm-none-eabi-gdb
(gdb) target remote localhost:3333
(gdb) file example2.elf
(gdb) load
(gdb) continue
^C
(gdb) dump binary memory example2.dump 0x10000000 0x10008000
(gdb) quit
$ 
```

The trace can be then inspected opening the `example2.dump` file with the *Tracealyzer for FreeRTOS* application.
