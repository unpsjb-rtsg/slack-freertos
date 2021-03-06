"""
Perform test evaluations of RTS on a mbed-compatible microcontroller.
Copyright (C) 2020  Francisco E. Páez

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""
import struct
import serial
import shutil
import glob
import os
import sys
import math
import xml.etree.cElementTree as et
from functools import reduce
from typing import TextIO
from argparse import ArgumentParser, FileType
from time import sleep


def lcm(rts: list) -> float:
    """ Real-time system hiperperiod (l.c.m) """
    return reduce(lambda x, y: (x * y) // math.gcd(x, y), [task["T"] for task in rts], 1)


def uf(rts: list) -> float:
    """ Real-time system utilization factor """
    return sum([float(task["C"]) / float(task["T"]) for task in rts])


def rta3(rts, rall=True) -> bool:
    import math

    a = [0] * len(rts)
    i = [0] * len(rts)

    for idx, task in enumerate(rts):
        a[idx] = task["C"]
        i[idx] = task["T"]

    t = rts[0]["C"]
    rts[0]["wcrt"] = t

    schedulable = True

    for idx, task in enumerate(rts[1:], 1):
        task_schedulable = True
        t_mas = t + task["C"]

        while task_schedulable:
            t = t_mas

            for jdx, jtask in zip(range(len(rts[:idx]) - 1, -1, -1), reversed(rts[:idx])):
                if t_mas > i[jdx]:
                    tmp = math.ceil(t_mas / jtask["T"])
                    a_tmp = tmp * jtask["C"]
                    t_mas += (a_tmp - a[jdx])

                    if t_mas > task["D"]:
                        if not rall:
                            return False
                        else:
                            schedulable = task_schedulable = False

                    a[jdx] = a_tmp
                    i[jdx] = tmp * jtask["T"]

            if t == t_mas:
                break

        task["wcrt"] = t

    return schedulable


def get_from_xml(file: TextIO, rts_id_list: list):
    """
    Retrieve the specified rts from a xml file
    :param file: file object handle
    :param rts_id: rts id
    :return: rts
    """
    # get an iterable
    context = et.iterparse(file.name, events=('start', 'end',))
    # turn it into a iterator
    context = iter(context)
    # get the root element
    event, root = next(context)

    current_id, rts_found = 0, False

    for rts_id in rts_id_list:
        rts = {"id": rts_id, "tasks": []}

        # read the xml, parse task-sets and simulate it
        for event, elem in context:
            if elem.tag == 'S':
                if event == 'start':
                    current_id = int(float(elem.get("count")))
                    if rts_id == current_id:
                        rts_found = True
                if event == 'end':
                    if rts_found:
                        break
                    elem.clear()

            if rts_found:
                if elem.tag == 'i':
                    if event == 'start':
                        task = elem.attrib
                        for k, v in task.items():
                            task[k] = int(float(v))
                        rts["tasks"].append(task)

            root.clear()

        analyze_rts(rts)
        yield rts
    del context


def analyze_rts(rts: dict) -> None:
    """
    Analyze the RTS and complete fields
    :param rts: rts
    :return: None
    """
    rts["u"] = uf(rts["tasks"])
    rts["lcm"] = lcm(rts["tasks"])
    rts["schedulable"] = rta3(rts["tasks"])


def mixrange(s: str) -> list:
    """
    Create a list of numbers from a string. Ie: "1-3,6,8-10" into [1,2,3,6,8,9,10]
    :param s: a string
    :return: a list of numbers
    """
    r = []
    for i in s.split(','):
        if '-' not in i:
            r.append(int(i))
        else:
            l, h = map(int, i.split('-'))
            r += range(l, h+1)
    return r


def send_rts_to_mbed(rts, ser):
    """
    Send the RTS task-model to the board throught the serial port.
    """
    # first send the task count
    ser.write(struct.pack('>i', len(rts)))

    # then send each task parameters
    for task in rts:
        ser.write(struct.pack('>i', task["C"]))
        ser.write(struct.pack('>i', task["T"]))
        ser.write(struct.pack('>i', task["D"]))


def get_int(ser):
    return int.from_bytes(ser.read(4), byteorder='big', signed=False)
        
        
def read_results_bytes(ser, taskcnt, instance_count):
    """
    Read the test results from the board throught the serial port.
    """
    # wait for the results to be ready
    wait_count = 0
    while True:
        if (ser.in_waiting > 0):
            break
        wait_count += 1
        if wait_count > 60:
            print("Error: no response for more than {0:}. Reopening serial connection.".format(60), file=sys.stderr)
            ser.close()
            sleep(5)
            ser.open()
            sleep(5)
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            ser.sendBreak(1)
            wait_count = 0
        sleep(1)
        
    results = []

    # first 4 bytes indicates if any error ocurred in the test
    fail = get_int(ser)

    if fail:
        return (fail, [], 0, 0, 0, 0)

    # the following 4 ints describe the test configuration
    test = get_int(ser)
    slack = get_int(ser)
    slack_method = get_int(ser)
    slack_k = get_int(ser)

    # the remaining data are the tasks individual results
    for _ in range(taskcnt):
        # first 4 bytes indicates the task number
        task_results = [get_int(ser)]

        # the following instance_count ints are the results
        for _ in range(instance_count):
            task_results.append(get_int(ser))

        results.append(task_results)

    return (0, results, test, slack, slack_method, slack_k)


def get_args():
    """ 
    Process the command line arguments.
    """
    parser = ArgumentParser()
    parser.add_argument("--port", help="Serial port of the mbed board.", default=None, type=str)
    parser.add_argument("--baudrate", help="Baudrate. Defaults to 9600.", default=9600, type=int)
    parser.add_argument("--timeout", help="Serial port timeout. Defaults to 5.", type=int, default=5)
    parser.add_argument("--taskcnt", help="Number of tasks of each RTS.", type=int, default=10)
    parser.add_argument("--instance-count", help="Number of instances per task.", type=int, default=10)
    parser.add_argument("--rts", type=str, help="RTS to test.")
    parser.add_argument("--xml", type=FileType('r'), help="XML file(s) with RTS to test.", nargs="+")

    return parser.parse_args()
    
    
def main():
    if not len(sys.argv) > 1:
        print("Missing arguments.", file=sys.stderr)
        print("Try '--help' for more information.", file=sys.stderr)
        exit(1)

    args = get_args()
    
    tests = { 1: 'cs', 2: 'ceils-floors', 3: 'ss-cost', 4: 'loops' }
    slack = { 0: 'noss', 1: 'ss' }
    slack_k = { 0: 'd', 1: 'k' }
    slack_method = { 0: 'fixed', 1: 'davis' }

    # create and configure serial connection to the mbed microcontroller
    try:
        ser = serial.Serial(port=args.port, baudrate=args.baudrate, timeout=args.timeout, write_timeout=5, xonxoff=True, dsrdtr=True)
    except serial.SerialException as err:
        print("SerialException: {0}.".format(err))
        sys.exit(1)
    except ValueError as err:
        print("ValueError: {0}.".format(err))
        sys.exit(1)

    # reset mbed board and wait
    ser.send_break(0.5)
    sleep(1)

    # keep count of the number of rts that could be tested (schedulable)
    ok_counter = 0    
    # fail counter, deadline, notsched, debug, stack, malloc
    errors = [0, 0, 0, 0, 0, 0]
    # rts which missed a deadline
    deadline_miss = []
    
    # range of rts to test in the xml files
    rts_list = mixrange(args.rts)
        
    # send rts parameters to the mbed board and wait for the results
    for xml_file in args.xml:
        print("XML file {0:}".format(xml_file.name), file=sys.stderr)
        for rts in get_from_xml(xml_file, rts_list):
            while(True):
                try:
                    print("Testing RTS {0}".format(rts["id"]), file=sys.stderr)
                    
                    ser.reset_input_buffer()
                    ser.reset_output_buffer()
                    ser.sendBreak(1)
                    sleep(2)
                    
                    # wait for the card to be ready
                    board_ready = False
                    wait_count = 0
                    while not board_ready:                        
                        if (ser.in_waiting > 0):
                            ok = get_int(ser);
                            if ok == 1234:                            
                                board_ready = True
                                break
                            else:
                                print("Error: the board seems to have a error when restarting.", file=sys.stderr)
                                break
                        wait_count += 1
                        if wait_count > 10:
                            print("Error: the board do not respond after restarting.", file=sys.stderr)
                            break                        
                        sleep(1)

                    if not board_ready:
                        continue                                            

                    send_rts_to_mbed(rts["tasks"], ser)
                    
                    # check if the card received all the data
                    board_ready = False
                    wait_count = 0
                    while not board_ready:
                        if (ser.in_waiting > 0):
                            ok = get_int(ser);
                            if ok == 4321:     
                                board_ready = True
                                break
                            else:
                                print("Error: something went wrong when sending the tasks to the board.", file=sys.stderr)
                                break
                        wait_count += 1
                        if wait_count > 10:
                            print("Error: the board do not respond after sending the tasks.", file=sys.stderr)
                            break
                        sleep(1)

                    if not board_ready:
                        continue
                    
                    test_result, results, test, s1, s2, s3 = read_results_bytes(ser, args.taskcnt, args.instance_count)

                    if test_result == 0:
                        if test not in tests.keys(): 
                            print("Error: invalid test key {0:}".format(test), file=sys.stderr)
                            continue
                        if s1 not in slack.keys(): 
                            print("Error: invalid slack key {0:}".format(test), file=sys.stderr)
                            continue
                        if s2 not in slack_method.keys(): 
                            print("Error: invalid slack method key {0:}".format(test), file=sys.stderr)
                            continue
                        if s3 not in slack_k.keys(): 
                            print("Error: invalid slack_k key {0:}".format(test), file=sys.stderr)
                            continue

                        if len(results) != args.taskcnt:
                            print("Error: tasks results is {0:}, should be {1:}".format(len(results), args.taskcnt), file=sys.stderr)
                            continue

                        for i, r in enumerate(results):
                            if len(r) != (args.instance_count + 1):
                                print("Error: instance results for task {0:} is {1:}, should be {2:}".format(r[0], len(r) - 1, args.instance_count), file=sys.stderr)
                                continue
                            if r[0] != i:
                                print("Error: corrupted results", file=sys.stderr)
                                continue
                    
                        for r in results:
                            print("{0}\t{1}\t{2}\t{3}\t{4}\t{5}".format(rts["id"], "\t".join(str(i) for i in r), 
                                os.path.basename(xml_file.name), slack[s1], slack_method[s2], slack_k[s3]))

                        ok_counter = ok_counter + 1
                        sys.stdout.flush()
                    else:
                        # increment the fail counters
                        errors[0] = errors[0] + 1
                        errors[test_result] = errors[test_result] + 1
                        deadline_miss.append(os.path.basename(xml_file.name))
                    break
                except KeyError as err:
                    print("KeyError: {0}".format(str(err)), file=sys.stderr)
                except ValueError as err:
                    print("ValueError: {0}".format(str(err)), file=sys.stderr)
                except IndexError as err:
                    print("IndexError: {0}".format(str(err)), file=sys.stderr)
                except (IOError, os.error) as err:
                    print("IOerror: {0}".format(str(err)), file=sys.stderr)

    # print summary
    print("{0} rts evaluated.".format(ok_counter), file=sys.stderr)
    print("{0} rts with errors:".format(errors[0]) if errors[0] > 0 else "No errors.", file=sys.stderr)
    if errors[0] > 0:
        print("\tdeadline missed: {0}".format(errors[1]), file=sys.stderr)
        print("\tnot schedulable: {0}".format(errors[2]), file=sys.stderr)    
        print("\tstack overflow: {0}".format(errors[4]), file=sys.stderr)
        print("\tmalloc failed: {0}".format(errors[5]), file=sys.stderr)  
        print("\tother error: {0}".format(errors[3]), file=sys.stderr)


if __name__ == '__main__':
    main()

