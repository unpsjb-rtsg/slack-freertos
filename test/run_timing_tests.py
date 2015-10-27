from __future__ import print_function

import serial
import shutil
import glob
import os
import sys
import csv
import util
from argparse import ArgumentParser
from time import sleep


def read_results(ser, taskcnt):
    results = []

    # first line indicates if an error ocurred
    fail = int(ser.readline().decode())

    if fail:
        return (fail, [], 0, 0, 0)
        
    slack = int(ser.readline().decode())
    slack_method = int(ser.readline().decode())
    slack_k = int(ser.readline().decode())

    for _ in range(taskcnt):
        results.append(ser.readline().decode())

    return (0, results, slack, slack_method, slack_k)


def get_args():
    """ Procesa argumentos de la linea de comandos """
    parser = ArgumentParser()
    parser.add_argument("--port", help="Puerto COM", default=None, type=str)
    parser.add_argument("--baudrate", help="Baudios", default=9600, type=int)
    parser.add_argument("--drive", help="mbed drive", type=str, default=None)    
    parser.add_argument("--binpath", help="bin directory", type=str)
    parser.add_argument("--timeout", help="individual test timeout", type=int, default=None)
    parser.add_argument("--savefile", help="Save file", type=str, default="")
    parser.add_argument("--append", help="Append results in save file", action="store_true")
    parser.add_argument("--cont", help="Continue from previous execution", action="store_true")
    parser.add_argument("--taskcnt", help="Number of tasks in the rts", type=int, default=10)

    return parser.parse_args()
    
    
def get_mbed_drive_and_port():
    mdslabels = ("mbed", "nucleo", "frdm")
    disks = util.get_logicaldisks()
    ports = util.get_serialports()
    
    mbed_drive = ""
    mbed_port = ""
    
    for item in disks:
        if (not item['name'] or not any([l in item['name'].lower() for l in mdslabels])):
            continue
        mbed_drive = item['disk']
        break
        
    for item in ports:
        if (not item['description'] or not any([l in item['description'].lower() for l in mdslabels])):
            continue
        mbed_port = item['port']
        break
        
    return [mbed_drive, mbed_port]
        
    
def main():
    args = get_args()
    
    mbeddp = get_mbed_drive_and_port()
    
    mbed_port = args.port
    if args.port is None:
        mbed_port = mbeddp[1]
    
    mbed_drive = ""
    if args.drive is None:
        mbed_drive = mbeddp[0]
    
    slack = { 0:'noss', 1:'ss' }
    slack_k = { 0:'d', 1:'k' }
    slack_method = { 0:'fixed', 1:'davis' }

    # create and configure serial connection to the mbed microcontroller
    ser = serial.Serial(port=mbed_port, baudrate=args.baudrate, timeout=args.timeout)
    ser.setBreak(True)

    # search for all the bin files previously generated
    bin_files = glob.glob("{0}/*.bin".format(args.binpath))

    if not bin_files:
        print("No binary files found!")
        return

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
        
    # bin files already tested
    tested_files = set()
    if args.cont:        
        with open(save_file_path, newline='') as f:
            reader = csv.reader(f, delimiter='\t', quotechar='"')
            for row in reader:
                if row:
                    tested_files.add(row[-4])      
    
    # copy each file into the mbed microcontroller, send a break through the
    # the serial port, and read the results.
    with open(save_file_path, mode) as save_file:
        for bin_file in bin_files:
            
            # if bin_file was already tested, skip it
            if args.cont:
                if os.path.basename(bin_file) in tested_files:
                    continue
            
            while(True):
                try:
                    print("Copying {0}".format(bin_file))
                    shutil.copy(bin_file, os.path.join(mbed_drive, "test.bin"))
                    print("Testing {0} ...".format(bin_file))
                    sleep(1)
                    ser.sendBreak()
                    r, results, s1, s2, s3 = read_results(ser, args.taskcnt)
                    if results:
                        # save results in save_file file
                        for r in results:
                            save_file.write("{1}\t{0}\t{2}\t{3}\t{4}\n".format(os.path.basename(bin_file), r.rstrip('\n'), slack[s1], slack_method[s2], slack_k[s3]))
                        ok_counter = ok_counter + 1
                    else:
                        # increment the fail counters
                        errors[0] = errors[0] + 1
                        errors[r] = errors[r] + 1
                        deadline_miss.append(os.path.basename(bin_file))
                    break
                except ValueError:
                    # something is wrong...
                    print("Error testing {0}, trying again ...".format(bin_file), file=sys.stderr)
                except IndexError:
                    # 
                    print("IndexError: {0}, trying again ...".format(r), file=sys.stderr)
                except (IOError, os.error) as why:
                    # an error with shutil.copy()
                    print("IOerror: {0}, trying again ...".format(str(why)), file=sys.stderr)
                    break
            sleep(1)

    # print results and end
    print("{0} rts evaluated.".format(ok_counter))
    print("{0} rts with errors:".format(errors[0]))
    print("\tdeadline missed: {0}".format(errors[1]))
    print("\tnot schedulable: {0}".format(errors[2]))    
    print("\tstack overflow: {0}".format(errors[4]))
    print("\tmalloc failed: {0}".format(errors[5]))  
    print("\tother error: {0}".format(errors[3]))
    print("Done!")


if __name__ == '__main__':
    main()
