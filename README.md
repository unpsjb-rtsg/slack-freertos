# Slack Stealing for FreeRTOS

A slack stealing framework for FreeRTOS.

## Overview

This project add Slack Stealing (SS) support into the [FreeRTOS Real-Time Operating System](http://www.freertos.org) for developing Mixed Critical Systems in a flexible way.

### Directories

* `board`: libraries required for compiling the example programs for multiple boards.
* `libs`: contains copies of the [FreeRTOS](http://www.freertos.org) source code, the [Percepio Tracealyzer](https://percepio.com/tz/) library and the [sAPI](https://github.com/epernia/sAPI) library.
* `examples`: example programs that use the Slack Stealing framework to schedule both periodic and aperiodic tasks.
* `slack`: slack stealing framework source code.
* `test`: source code, scripts and other resources for running several tests that evaluate the perfomance of the Slack Stealing framework according to different metrics.

## Publications

* Páez F.E., Urriza J.M., Orozco J.D. (2021) *Idle Time Administration on FreeRTOS Using Slack Stealing*. In: Pesado P., Eterovic J. (eds) Computer Science – CACIC 2020. CACIC 2020. Communications in Computer and Information Science, vol 1409. Springer, Cham. https://doi.org/10.1007/978-3-030-75836-3_19
* *Administración del Tiempo Ocioso Mediante Slack Stealing en FreeRTOS*, Francisco E. Páez, José M. Urriza, Javier D. Orozco. XXVI Congreso Argentino de Ciencias de la Computación (CACIC) (Modalidad virtual, 5 al 9 de octubre de 2020). ISBN 978-987-4417-90-9, pp. 629-638. [Download](http://sedici.unlp.edu.ar/handle/10915/113243).
* *Métodos de Slack Stealing en FreeRTOS*, Francisco E. Páez, José M. Urriza, Ricardo Cayssials, Javier D. Orozco. 44 Jornadas Argentinas de Informática (JAIIO) – SII 2015. ISSN 2451-7542, pp. 216-227. [Download](http://44jaiio.sadio.org.ar/sites/default/files/sii216-227.pdf).

## COPYING

This software is licensed under the GNU General Public License v2.0. A copy of the license can be found in the `LICENSE` file.

FreeRTOS is Copyright (C) 2010 Real Time Engineers Ltd., and is licensed under a modified GNU General Public License (GPL).

The Tracealyzer Recorder Library is Copyright (C) Percepio AB, 2014.

The mbed Microcontroller Library is Copyright (c) 2006-2013 ARM Limited, and is licensed under the Apache License, Version 2.0.

The LPCOpen library is Copyright(C) NXP Semiconductors, 2012.
