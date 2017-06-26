# Example 3
This example project implements a *Rate Monotonic Scheduling* policy and performs an online slack calculation. The available slack is used to schedule an aperiodic task, which is ready to run at random times.

This program is similar to that of example 2, but generate a detailed trace of the system execution using the [Percepio Tracealyzer](https://percepio.com/tz/) library.

By default the example uses the *Snapshot Mode*, which keeps the trace in a buffer, allowing for saving a snapshot at any point by making a memory dump.

For example using CMSIS-DAP, a memory dump could be done starting the `openocd` gdb server, with the EDU-CIAA-NXP board connected, and then executing the following commands in `arm-none-eabi-gdb` debugger session:
```bash
$ arm-none-eabi-gdb
(gdb) target remote localhost:3333
(gdb) file example3.elf
(gdb) load
(gdb) continue
^C
(gdb) dump binary memory example3.dump 0x1FFF0000 0x2002FFFF
(gdb) quit
$ 
```

The trace can be then inspected opening the `example3.dump` file with the *Tracealyzer for FreeRTOS* application.
