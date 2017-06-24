# Example Programs
These examples runs on the mbed LPC 1768 development board. All the examples creates a set of four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*, scheduled using a *Rate Monotonic Scheduling* (RMS) policy.

To choose which example project build, change the `TARGET` parameter in the root `Makefile` to one of these values:
* `example1`: Rate Monotonic scheduling, with online slack calculation.
* `example2`: Rate Monotonic scheduling, using Slack Stealing to schedule and additional high-priority aperiodic task.
* `example3`: Same as example 2, but it uses the Tracealizer library to generate a execution trace.

The `utils` directory contains utility functions used by these examples.

