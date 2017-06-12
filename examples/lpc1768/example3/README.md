# Slack Stealing example 3
Example project that uses a *Rate Monotonic* scheduler with *Slack Stealing*. The available slack is used to schedule an aperiodic task, which is ready to run at random times.

The example is similar to the example2, but uses the Tracealizer library to generate a detailed trace of the system execution.

For default the example uses the *Snapshot Mode*, which keeps the trace in a buffer, allowing for saving a snapshots at any point by saving the RAM buffer. 

To manually save a RAM dump using `pyocd-gdbserver` and `arm-none-eabi-gdb` debugger:

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

Then you can inspect the trace using the *Tracealyzer for FreeRTOS* application.