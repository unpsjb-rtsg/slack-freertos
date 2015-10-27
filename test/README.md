# Tests

This folder contains code for executing the tests performed in the paper.

## Required software

* ARM C Compiler. We recommend GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* Mbed library.
* FreeRTOS source code.
* Python 2.7 (3.x should work fine too).
* Python modules cog (http://nedbatchelder.com/code/cog/) and pyserial (http://pyserial.sourceforge.net/).

## Create a test

1. Create a separate directory where all the required test files should be placed. In this example, we will use the Test1 as the name for this folder.

2. Copy into the Test1 directory the file the Makefile file. Update the variables GCC\_BIN, MBED\_DIR, FREERTOS\_DIR and SLACK\_DIR with the paths to the ARM C compiler, the mbed library, the FreeRTOS source code and the slack stealing modification.

3. Copy the FreeRTOSConfig.h file into the Test1 directory.

4. Execute the generate\_cpps.py script in order to generate the source code and binary files. For example:

```
python generate\_cpps.py --template main.cpp --xmlpath /rts/xmldir/ --srcpath Test1 --start 1 --count 50 --taskcnt 10 --releasecnt 10 --testsched --bins
```

5. Execute the run\_timing\_tests.py script to perform the tests. For example:

```
python run\_timing\_tests.py --port 4 --baudrate 9600 --drive h --binpath Test1
```
