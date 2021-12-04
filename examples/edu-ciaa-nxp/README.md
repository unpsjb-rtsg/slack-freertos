# Example Programs
These examples runs on the [EDU-CIAA-NXP](http://www.proyecto-ciaa.com.ar/devwiki/doku.php?id=desarrollo:edu-ciaa:edu-ciaa-nxp) development board.

To build an example project change the `TARGET` parameter to `edu-ciaa-nxp` and set `APP_NAME` to one of these values:
* `example1`: Schedule three periodic using the *Rate Monotonic Scheduling* (RMS) policy with online slack calculation.
* `example2`: Use the available slack to schedule a high-priority aperiodic task.
