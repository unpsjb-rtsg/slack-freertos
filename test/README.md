# mbed LPC1768 tests

This directory contains the source code, scripts and other resources needed to run several performance tests. This evaluation are designed to run on a mbed LPC1768 board.

## Available tests

The available tests are:

1. Execution cost measured in CPU cycles of the `vTaskDelayUntil()` kernel function.
2. Amount of ceil and floor operations performed by the selected Slack Stealing method.
3. Execution cost of the selected Slack Stealing method measured in CPU cycles.
4. Amount of for and while loops performed by the selected Slack Stealing method.

Modify the `configKERNEL_TEST` clause in the `FreeRTOSConfig.h` file to specify which test will be implemented in the BIN files.

## Required software and files

Required software:
* ARM C Compiler, for example GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* Mbed library. A copy is provided in the `libs/mbed` directory.
* FreeRTOS source code. Copies of FreeRTOS are provided in the `libs/FreeRTOS` directory.
* Python 2.7 (3.x should work fine too).
* Python modules *cog* (http://nedbatchelder.com/code/cog/) and *pyserial* (http://pyserial.sourceforge.net/).

Also, one or more xml files with tasks groups, generated with the RTTSG tool (available at http://goo.gl/GqQPUK) are needed. These files are used to generate the periodic tasks that each program will execute in the microcontroller. A set of example xml files could be found in the `rts` directory, with groups of 10 tasks and utilization factor from 10% to 90%.

## Python scripts

Two python scripts are provided to generate and run the tests:

* `generate_cpps.py`: parse the xml files and generate bin files into the specificied directory. The most important options are (use `--help` to see all the available options):
  * `--slack`: if present the generated programs will implement Slack Stealing.
  * `--slackmethod`: specify the Slack Stealing algorithm to implement. Currently you could choose between `fixed` (Urriza et. al.) and `davis` (Davis et. al.) slack calculation methods.
  * `--slackcalc`: indicates if the slack stealing algorithm should be executed each time that a task instance finish (`ss`) or only at the system startup (`k`). 

* `run_timing_tests.py`: perform the execution of the tests, copying each BIN file in the specified mbed board, reading back the results through the serial port and saving them in a file.

## Create a test

Follow the next steps to prepare the required files for running a test:

1. Create a dedicated directory for the test under the `tests` directory. All the required files will be placed in this directory. For this example, we will use `Test1` as the name for this folder.

2. Copy the `Makefile` file into the `Test1` directory. Change the variable `GCC_BIN` with the path to the ARM C Compiler `bin` directory, if the compiler was not added to the system or user path.  Next, update the variables `MBED_DIR`, `FREERTOS_DIR` and `SLACK_DIR` if needed.

3. Copy the `FreeRTOSConfig.h` file into the `Test1` directory. Assing to `configKERNEL_TEST` the number of the test to execute.

4. Finally, run the `generate_cpps.py` script to generate the source code and binary files. For example, to create binary and source code files for the first 10 real-time systems in each XML found in the `rts` directory: 

```
python generate_cpps.py --template main.cpp --xmlpath ./rts --srcpath ./tests/Test1 --start 1 --count 10 --taskcnt 10 --releasecnt 10 --testsched --bins --cpps --slack
```

## Running the test

To execute the test run the `run_timing_tests.py` script. The script should detect the mbed board connected to the PC, so the only required parameter is `binpath`, which must point to the directory where the BIN files are:

```
python run_timing_tests.py --binpath ./stests/Test1
```

If the mbed board is not detected, or if more than one board is present, use the `port` and `drive` parameters to indicate the specific port and drive letter (or mount point). For example, if the board was assigned the COM4 port and the H drive letter:

```
python run_timing_tests.py --port COM4 --baudrate 9600 --drive H: --binpath tests/Test1
```

The `baudrate` argument is optional, as 9600 is the default value used when generating the binary files.

