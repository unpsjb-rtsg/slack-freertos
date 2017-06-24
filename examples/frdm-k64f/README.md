# Example Programs
These examples runs on the FRDM-K64F development board.

To choose which example project build, change the `TARGET` parameter in the `Makefile.mine` file to one of these values:
* `example1`: Four periodic real-time tasks, scheduled using a Rate Monotonic Scheduling (RMS) policy. Each task send through the serial port the available system slack.
* `example2`: Same as exmaple 1, with an additional aperiodic tasks, that is executed only when slack exists.
* `example3`: Same as example 2, but with two aperiodic tasks, and uses the Tracealizer library to generate a trace.

All the examples implements a system with four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*.

The `utils` directory contains utility functions used by these examples.

