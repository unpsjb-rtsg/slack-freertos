# Example Programs
These examples runs on the mbed LPC 1768 microcontroller.

To choose which example project build, change the `TARGET` parameter in the root `Makefile` to one of these values:
* `example1`: Rate Monotonic scheduling + Slack Stealing.
* `example2`: Rate Monotonic scheduling + Slack Stealing + An aperiodic task.
* `example3`: Same as example 2, but it uses the Tracealizer library to generate a trace.

All the examples implements a system with four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*.

The `utils` directory contains utility functions used by these examples.

