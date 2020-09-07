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
* The original design (v1.0.0) is documented in *Métodos de Slack Stealing en FreeRTOS*, Francisco E. Páez, José M. Urriza, Ricardo Cayssials, Javier D. Orozco. 44 Jornadas Argentinas de Informática (JAIIO) – SII 2015. ISSN 2451-7542, pp. 216-227.

## COPYING
This software is licensed under the GNU General Public License v2.0. A copy of the license can be found in the `LICENSE` file.

FreeRTOS is Copyright (C) 2010 Real Time Engineers Ltd., and is licensed under a modified GNU General Public License (GPL).

The Tracealyzer Recorder Library is Copyright (C) Percepio AB, 2014.

The mbed Microcontroller Library is Copyright (c) 2006-2013 ARM Limited, and is licensed under the Apache License, Version 2.0.

The LPCOpen library is Copyright(C) NXP Semiconductors, 2012.
