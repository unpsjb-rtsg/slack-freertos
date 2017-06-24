# Slack Stealing example 3
This example project implements a *Rate Monotonic* scheduling policy, with *Slack Stealing*. The available slack is used to schedule two aperiodic tasks, which are ready to run at random times.

This program is similar to that of example 2, but generate a detailed trace of the system execution using the Tracealyzer library.

By default the example uses the *Snapshot Mode*, which keeps the trace in a buffer, allowing for saving a snapshot at any point by making a memory snapshot.

For example using CMSIS-DAP, a memory dump could be done starting the `pyocd-gdbserver` with the lpc1768 board connected, and then executing the following commands in `arm-none-eabi-gdb` debugger session:

```bash
$ arm-none-eabi-gdb
(gdb) target remote localhost:3333
(gdb) file example3.elf
(gdb) load
(gdb) continue
^C
(gdb) dump binary memory example3.dump 0x10000000 0x10008000
(gdb) quit
$ 
```

The trace can be then inspected opening the `example3.dump` file with the *Tracealyzer for FreeRTOS* application.