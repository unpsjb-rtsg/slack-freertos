# Example programs
These examples runs on the [FRDM-K64F](http://www.nxp.com/products/software-and-tools/hardware-development-tools/freedom-development-boards/freedom-development-platform-for-kinetis-k64-k63-and-k24-mcus:FRDM-K64F) development board with the [mbed library](https://developer.mbed.org/platforms/FRDM-K64F/). All the examples creates a set of four periodic tasks, with periods of 3000, 4000, 6000 and 12000 *ticks*, scheduled using the *Rate Monotonic Scheduling* (RMS) policy.

To build an example project change in the `Makefile.mine` file the `TARGET` parameter to `frdm-k64f` and set `APP_NAME` to one of these values:
* `example1`: Perform online slack calculation.
* `example2`: Use the available slack to schedule a set of high-priority aperiodic tasks.


