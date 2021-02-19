# Example programs

These examples runs on the [mbed LPC 1768](https://developer.mbed.org/platforms/mbed-LPC1768/) development board. All the examples creates a set of four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*, scheduled using a *Rate Monotonic Scheduling* (RMS) policy.

To build an example project change in the `Makefile.mine` file the `TARGET` parameter to `lpc1768` and set `APP_NAME` to one of these values:

* `example1`: Perform online slack calculation.
* `example2`: Schedule a set of high-priority aperiodic tasks using the available slack.
* `example3`: Use the available slack to schedule a set of high-priority aperiodic tasks.
* `example4`: Same as example 1, but for FreeRTOS v10 or later.
* `example5`: Similar to example 2, but for FreeRTOS v10 or later.
