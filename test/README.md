# mbed LPC1768 tests

This directory contains the source code, scripts and other resources needed to run several performance tests. This evaluations run on the mbed LPC1768 board.

## Available tests

The available tests are:
1. Cost in CPU cycles of the vTaskDelayUntil() kernel function.
2. Amount of ceil and floor operations performed by the Slack Stealing method.
3. Execution cost of the Slack Stealing method in CPU cycles.
4. Amount of for and while loops required by the Slack Stealing method.

Modify the `configKERNEL_TEST` clause in the `FreeRTOSConfig.h` file to specify which test will be implemented in the BIN files.

## Required software and files

Required software:
* ARM C Compiler. We recommend GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* Mbed library. A copy is provided in the `libs/mbed` directory.
* FreeRTOS source code. A copy of FreeRTOS is provided in the `libs/FreeRTOS` directory.
* Python 2.7 (3.x should work fine too).
* Python modules *cog* (http://nedbatchelder.com/code/cog/) and *pyserial* (http://pyserial.sourceforge.net/).

Also, one or more xml files with tasks groups, generated with the RTTSG tool (available at http://goo.gl/GqQPUK) are needed. These files are used to generate the periodic FreeRTOS tasks that each program will run in the mbed microcontroller. A set of example files could be found in the `rts` directory, with groups of 10 tasks and utilization factor from 10% to 90%.

## Python scripts

Two python scripts are provided to generate and run the tests:

* `generate_cpps.py`: read the XML files and generate the BIN files into a specificied directory. Run the script with `--help` to see the available options. The most important are:
  * `slack`: if present the generated programs will implement Slack Stealing.
  * `slackmethod`: the Slack Stealing algorithm to implement. Currently you could choose between `fixed` (Urriza et. al.) and `davis` (Davis et. al.) methods.
  * `slackcalc`: indicates if the slack stealing algorithm should be executed at the finalization of a task instance (`ss`) or only at the system startup (`k`).

* `run_tests.py`: execute the tests, copying each BIN file in the mbed board, reading back the results and saving it in a file.

## Create a test

1. Create a dedicated directory under the `tests` directory. All the required files will be placed in this directory. In this example, we will use `Test1` as the name for this folder.

2. Copy into the `Test1` directory the `Makefile` file. Change the variable `GCC_BIN` with the path to the ARM C Compiler `bin` directory. Next, update the variables `MBED_DIR`, `FREERTOS_DIR` and `SLACK_DIR` if needed.

3. Copy the `FreeRTOSConfig.h` file into the `Test1` directory. Change the `configKERNEL_TEST` with the number of the test to perform.

4. Finally, execute the `generate_cpps.py` script in order to generate the source code and binary files. For example, in order to create binary files for the first 10 real-time systems in each XML in `rts`, 

```
python generate_cpps.py --template main.cpp --xmlpath rts/ --srcpath tests/Test1 --start 1 --count 10 --taskcnt 10 --releasecnt 10 --testsched --bins --cpps --slack
```

## Running the test

To execute the tests run the `run_timing_tests.py` script. The script should detect the mbed board connected to the PC, so the only required parameter is `binpath`, which must point to the directory where the BIN files are:

```
python run_timing_tests.py --binpath tests/Test1
```

If the mbed board is not detected, or if more than one mbed is present, use the `port` and `drive` parameters. For example, if the mbed board was assigned the COM4 port and the H drive letter:

```
python run_timing_tests.py --port COM4 --baudrate 9600 --drive H: --binpath tests/Test1
```

The `baudrate` argument is optional, as 9600 is the default value used.

