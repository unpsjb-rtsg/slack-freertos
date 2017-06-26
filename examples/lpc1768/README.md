# Example Programs
These examples runs on the mbed LPC 1768 development board. All the examples creates a set of four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*, scheduled using a *Rate Monotonic Scheduling* (RMS) policy.

To build an example project change the `TARGET` parameter to `lpc1768` and set `APP_NAME` to one of these values:
* `example1`: Perform online slack calculation.
* `example2`: Use the available slack to schedule and additional high-priority aperiodic task.
* `example3`: Same as example 2, but it adds the Tracealizer library to generate a execution trace.

The `utils` directory contains utility functions used by these examples.

