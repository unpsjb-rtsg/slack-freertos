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
    # send task count
    ser.write(struct.pack('>i', len(rts)))

    # send task parameters
    for task in rts:
        ser.write(struct.pack('>i', task["C"]))
        ser.write(struct.pack('>i', task["T"]))
        ser.write(struct.pack('>i', task["D"]))


def read_results(ser, taskcnt):
    results = []

    # first line indicates if an error ocurred
    fail = int(ser.readline().decode().rstrip())

    if fail:
        return (fail, [], 0, 0, 0, 0)
        
    test = int(ser.readline().decode().rstrip())
    slack = int(ser.readline().decode().rstrip())
    slack_method = int(ser.readline().decode().rstrip())
    slack_k = int(ser.readline().decode().rstrip())

    for _ in range(taskcnt):
        results.append(ser.readline().decode().rstrip())

    return (0, results, test, slack, slack_method, slack_k)


def get_args():
    """ Procesa argumentos de la linea de comandos """
    parser = ArgumentParser()
    parser.add_argument("--port", help="COM port", default=None, type=str)
    parser.add_argument("--baudrate", help="Baudios", default=9600, type=int)
    parser.add_argument("--timeout", help="individual test timeout", type=int, default=25)
    parser.add_argument("--savefile", help="Save file", type=str, default="")
    parser.add_argument("--append", help="Append results in save file", action="store_true")
    parser.add_argument("--cont", help="Continue from previous execution", action="store_true")
    parser.add_argument("--taskcnt", help="Number of tasks in the rts", type=int, default=10)
    parser.add_argument("--instance-count", help="Number of instances per task.", type=int, default=10)
    parser.add_argument("--rts", type=str, help="RTS inside file(s) to test.")
    parser.add_argument("--xml", type=FileType('r'), help="XML file(s) with RTS to test.", nargs="+")

    return parser.parse_args()
    
    
def main():
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

    # path name for the text file where store the results
    if args.savefile:
        save_file_path = args.savefile
    else:
        save_file_path = os.path.join(args.binpath, "results.txt")

    # keep count of the number of rts that could be tested (schedulable)
    ok_counter = 0    
    # fail counter, deadline, notsched, debug, stack, malloc
    errors = [0, 0, 0, 0, 0, 0]
    # rts which missed a deadline
    deadline_miss = []
    
    if args.append:
        mode = "a"
    else:
        mode = "w"

    # range of rts to test in the xml files
    rts_list = mixrange(args.rts)
        
    # send rts parameters to the mbed board and wait for the results
    with open(save_file_path, mode) as save_file:
        for xml_file in args.xml:
            print("XML file {0:}".format(xml_file.name))
            for rts in get_from_xml(xml_file, rts_list):
                while(True):
                    try:
                        print("Testing RTS {0}".format(rts["id"]))
                        
                        ser.reset_input_buffer()
                        ser.reset_output_buffer()
                        ser.sendBreak(0.5)
                        sleep(1)

                        send_rts_to_mbed(rts["tasks"], ser)
                        
                        test_result, results, test, s1, s2, s3 = read_results(ser, args.taskcnt)
                        
                        if test_result == 0:
                            if results:
                                if len(results) != args.taskcnt:
                                    print("Error: tasks results is {0:}, should be {1:}".format(len(results), args.taskcnt))
                                    continue
                                
                                for r in results:
                                    if len(r.split()) != (args.instance_count + 1):
                                        print("Error: instance results for task {0:} is {1:}, should be {2:}".format(r.split()[0], len(r.split()) - 1, args.instance_count))
                                        continue

                                for r in results:
                                    save_file.write("{0}\t{1}\t{2}\t{3}\t{4}\t{5}\n".format(rts["id"], r.rstrip('\n\r'), os.path.basename(xml_file.name), slack[s1], slack_method[s2], slack_k[s3]))
                                ok_counter = ok_counter + 1
                                save_file.flush()
                        else:
                            # increment the fail counters
                            errors[0] = errors[0] + 1
                            errors[test_result] = errors[test_result] + 1
                            deadline_miss.append(os.path.basename(xml_file.name))
                        break
                    except ValueError as err:
                        print("ValueError: {0}".format(str(err)), file=sys.stderr)
                    except IndexError as err:
                        print("IndexError: {0}".format(str(err)), file=sys.stderr)
                    except (IOError, os.error) as err:
                        print("IOerror: {0}".format(str(err)), file=sys.stderr)
                        break

    # print results and end
    print("Results saved in {0}".format(save_file_path))
    print("{0} rts evaluated.".format(ok_counter))
    print("{0} rts with errors:".format(errors[0]) if errors[0] > 0 else "No errors.")
    if errors[0] > 0:
        print("\tdeadline missed: {0}".format(errors[1]))
        print("\tnot schedulable: {0}".format(errors[2]))    
        print("\tstack overflow: {0}".format(errors[4]))
        print("\tmalloc failed: {0}".format(errors[5]))  
        print("\tother error: {0}".format(errors[3]))


if __name__ == '__main__':
    main()

