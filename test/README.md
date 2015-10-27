# Tests

This folder contains code for executing the tests performed in the paper.

## Required software

* ARM C Compiler. We recommend GNU Tools for ARM (https://launchpad.net/gcc-arm-embedded).
* Mbed library.
* FreeRTOS source code.
* Python 2.7 (3.x should work fine too).
* Python modules cog (http://nedbatchelder.com/code/cog/) and pyserial (http://pyserial.sourceforge.net/).

## Execute a test

1. Create a separate directory where all the required test files should be placed. In this example, we will use the Test1 as the name for this folder.

2. Copy into the Test1 folder the file Makefile\_cpps, as Makefile. Update the variables GCC\_BIN, MBED\_DIR and FREERTOS\_DIR, with the paths to the ARM C compiler, the mbed library and the FreeRTOS source code.

3. Create the directories Test1/FreeRTOS and Test1/F

3) Crear el directorio Test1/FreeRTOS y Test1/FreeRTOS/portable, y copiar en el primero el archivo Makefile_freertos, con el nombre Makefile. Actualizar la variable GCC_BIN con el directorio del compilador GCC, y VPATH con el path al código fuente de FreeRTOS.

4) Copiar FreeRTOSConfig_base.h como FreeRTOSConfig.h. Modificar seg�n se requiera.

5) Ejecutar generate_cpps.py para generar los archivos CPPs y los binarios. Ejemplo:

python3 generate_cpps.py --template main.cpp --xmlpath /rts/xmldir/ --srcpath Test1 --start 1 --count 50 --taskcnt 10 --releasecnt 10 --testsched --bins

6) Ejecutar run_timing_test.py para copiar los bins a las mbed y ejecutar la prueba.

python3 run_timing_tests.py --port 4 --baudrate 9600 --drive h --binpath Test1