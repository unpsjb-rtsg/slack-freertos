# Perfomance evaluations

This directory contains the source code, scripts and other resources needed to run several tests that evaluate the perfomance of the Slack Stealing framework according to different metrics. These evaluations are designed to run on a mbed LPC1768 development board.

## Available tests

The following evaluations are available:

1. Execution cost measured in CPU cycles of the `vTaskDelayUntil()` kernel function.
2. Amount of ceil and floor operations performed by the selected Slack Stealing method.
3. Execution cost of the selected Slack Stealing method measured in CPU cycles.
4. Amount of `for` and `while` loops performed by the selected Slack Stealing method.

Modify the `configKERNEL_TEST` clause in the `FreeRTOSConfig.h` file to specify which test will be implemented in the BIN files.

## Required software

The following tools are required for generating and running the tests:
* An ARM C Compiler, for example the GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* The mbed library. A copy is provided in the `libs/mbed` directory.
* The FreeRTOS source code. Copies of supported FreeRTOS versions are provided in the `libs/FreeRTOS` directory.
* Python 2.7 installed (3.x should work fine too).
* Python modules *cog* (http://nedbatchelder.com/code/cog/), for generating the CPP files, and *pyserial* (http://pyserial.sourceforge.net/), for getting back the results from the mbed board through the serial port.

## Required resources

One or more xml files with tasks groups, generated with the RTTSG tool (available at http://goo.gl/GqQPUK) are needed. These files are used to generate the periodic tasks that each program will execute in the microcontroller. A set of example xml files could be found in the `rts` directory, with groups of 10 tasks and utilization factor from 10% to 90%.

## Python scripts

Two python scripts are provided to generate the CPP files, and run the tests:

* `generate_cpps.py`: get the specified number of task-sets from the xml files, and generate cpp files into the specificied directory and, optionaly, compile each source code file. Use the `--help` option to see all the available parameters. The most important options are:
  * `--slack`: if present the generated programs will implement Slack Stealing.
  * `--slackcalc`: indicates if the slack stealing algorithm should be executed each time that a task instance finish (`ss`) or only at the system startup (`k`).
  * `--slackmethod`: specify the Slack Stealing algorithm to implement. Currently you could choose between `fixed` (Urriza et. al.) and `davis` (Davis et. al.) slack calculation methods.
  * `--freertos`: which FreeRTOS version to use for building the binaries. 

* `run_timing_tests.py`: perform the execution of the tests, copying each BIN file in the specified mbed board, reading back the results through the serial port and saving them in a file.

## Create a test

Follow the next steps to prepare the required files for running a test:

1. First create a directory for the test under the `tests` directory. All the required files will be placed in this directory. For this example, `Test1` will be used as the name for this folder.

2. Copy the `Makefile` file into the `Test1` directory. If the path to a suitable compiler was not added to the system or user path, specify it in the variable `GCC_BIN`. Next, update the variables `MBED_DIR`, `FREERTOS_DIR` and `SLACK_DIR` if needed.

3. Next, copy the `FreeRTOSConfig.h` file into the `Test1` directory.

4. Run the `generate_cpps.py` script to generate the source code and binary files. For example, to create binary and source code files for the first 10 real-time systems in each XML found in the `rts` directory:
```
python generate_cpps.py --srcpath ./tests/Test1 --taskcnt 10 --releasecnt 10 --testsched --slack 10 ./rts/*.xml
```

Once the CPPs files are generated, the binary files could be recompiled executig `make` in the `Test1` directory. 

## Running the test

To execute the test run the `run_timing_tests.py` script. Use the `port` and `drive` parameters to indicate the serial port and drive letter (or mount point) assigned to the board, and specify the directory where the BIN files are with the `binpath` parameter. The results are saved by default in the file `binpath\results.txt`. For example:
```
python run_timing_tests.py --port COM4 --baudrate 9600 --drive H: --binpath tests/Test1
```
The `baudrate` argument is optional, as 9600 is the default value used when generating the binary files. Use the `--help` parameter to see all the available options.


