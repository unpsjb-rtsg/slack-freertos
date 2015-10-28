# Tests

Source code and scripts for executing the tests performed for the paper.

## Required software

* ARM C Compiler. We recommend GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* Mbed library.
* FreeRTOS source code.
* Python 2.7 (3.x should work fine too).
* Python modules cog (http://nedbatchelder.com/code/cog/) and pyserial (http://pyserial.sourceforge.net/).

## Create a test

1. Create a dedicated directory under the `tests` directory. All the required files will be placed in this directory. In this example, we will use `Test1` as the name for this folder.

2. Copy into the `Test1` directory the `Makefile` file. Change the variable `GCC_BIN` with the path to the ARM C Compiler `bin` directory. Next, update the variables `MBED_DIR`, `FREERTOS_DIR` and `SLACK_DIR` if needed.

3. Copy the `FreeRTOSConfig.h` file into the `Test1` directory.

4. Finally, execute the `generate_cpps.py` script in order to generate the source code and binary files. For example, in order to create binary files for the first 10 real-time systems in each XML file in `/rts/xmldir/`:

```
python generate_cpps.py --template main.cpp --xmlpath /rts/xmldir/ --srcpath tests/Test1 --start 1 --count 10 --taskcnt 10 --releasecnt 10 --testsched --bins --cpps
```

## Running the test

Execute the `run_timing_tests.py` script to perform the tests. For example, if the mbed board was assigned the COM4 port and the H drive letter:

```
python run_timing_tests.py --port COM4 --baudrate 9600 --drive H: --binpath tests/Test1
```

The script can detect the mbed board, and then only the `binpath` parameter is required:

```
python run_timing_tests.py --binpath tests/Test1
```