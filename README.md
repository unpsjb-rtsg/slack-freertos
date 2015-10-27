# Slack Stealing for FreeRTOS.
Patch for using a Slack Stealing enabled scheduler in FreeRTOS.

## Overview
This project implements a Slack Stealing (SS) scheduler, in order to support Mixed Critical Systems in a flexible way, in the FreeRTOS Real-Time Operating System (www.freertos.org).

The system design is documented in: *Métodos de Slack Stealing en FreeRTOS*, Francisco E. Páez, José M. Urriza, Ricardo Cayssials, Javier D. Orozco. 44 Jornadas Argentinas de Informática (JAIIO) – SII 2015. ISSN 2451-7542, pp. 216-227.

The `libs` directory contains a copy of the FreeRTOS source code for the ARM Cortex-M3 port. Also, a copy of the mbed Microcontroller Library for the mbed LPC 1768 board can be found in the `mbed` directory.

## Examples
See the `examples` directory for some example programs.

## COPYING
This software is licensed under the GNU General Public License v2.0. A copy of the license can be found in the `LICENSE` file.

FreeRTOS is Copyright (C) 2010 Real Time Engineers Ltd., and is licensed under a modified GNU General Public License (GPL). See: http://www.freertos.org/a00114.html

The Tracealyzer v3.0.2 Recorder Library is Copyright (C) Percepio AB, 2014.

The mbed Microcontroller Library is Copyright (c) 2006-2013 ARM Limited, and is licensed under the Apache License, Version 2.0.
