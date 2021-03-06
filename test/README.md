# Perfomance evaluations

This directory contains the source code, scripts and other resources needed to evaluate the perfomance of the Slack Stealing framework. These evaluations are designed to run on a mbed LPC1768 development board.

## Available tests

The following evaluations are available:

1. Execution cost measured in CPU cycles of the `vTaskDelayUntil()` kernel function.
2. Amount of ceil and floor operations performed by the selected Slack Stealing method.
3. Execution cost of the selected Slack Stealing method measured in CPU cycles.
4. Amount of `for` and `while` loops performed by the selected Slack Stealing method.

Modify the `KERNEL_TEST` option in the `Makefile.config` file to specify which test will be executed.

## Required software

The following tools are required for generating and running the tests:

* An ARM C Compiler, for example the [GNU Tools for ARM](https://launchpad.net/gcc-arm-embedded).
* The mbed library. A copy is provided in the `libs/mbed` directory.
* The FreeRTOS source code. Copies of supported FreeRTOS versions are provided in the `libs/FreeRTOS` directory.
* Python 3.
* [pyserial](http://pyserial.sourceforge.net/) for communicating with the mbed board through the serial port.

## Required resources

One or more xml files with tasks groups, generated with the [RTTSG tool](http://goo.gl/GqQPUK) are needed. These files are used to generate the periodic tasks that each program will execute in the microcontroller. A set of example xml files could be found in the `rts` directory, with groups of 10 tasks and utilization factor from 10% to 90%.

## Create and running a test

Follow the next steps to prepare the required files for running a test:

1. First edit the `Makefile.config` file and specify the desired test and other configuration.

2. Execute `make`, which compiles the test program and generates a `main.bin` file.

3. Copy the `main.bin` file into the mbed board.

4. Run the `run-test.py` script, passing the serial port of the mbed board, the task-set task count, the number of instances to evaluate per task, the selected rts and the xml files. The results are sended to the `stdout`. Use `--help` for more details.

For example:
```
$ python3 run-test.py --port COM4 --taskcnt 10 --instance-count 30 --rts 1-10 --xml rts/rtts_u10_n20.xml > results.txt
```
