import serial
import shutil
import glob
import os
import sys
import csv
from argparse import ArgumentParser
from time import sleep

from pyocd.core.helpers import ConnectHelper
from pyocd.flash.file_programmer import FileProgrammer
from pyocd.core.exceptions import TransferError
from pyocd.probe.pydapaccess.dap_access_api import DAPAccessIntf

def read_results(ser, taskcnt):
    results = []
    
    # first line indicates if an error ocurred
    fail = int(ser.readline().decode().rstrip())

    if fail:
        return (fail, [], 0, 0, 0)
        
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
    parser.add_argument("--drive", help="mbed drive", type=str, default=None)    
    parser.add_argument("--binpath", help="bin directory", type=str)
    parser.add_argument("--timeout", help="individual test timeout", type=int, default=25)
    parser.add_argument("--savefile", help="Save file", type=str, default="")
    parser.add_argument("--append", help="Append results in save file", action="store_true")
    parser.add_argument("--cont", help="Continue from previous execution", action="store_true")
    parser.add_argument("--taskcnt", help="Number of tasks in the rts", type=int, default=10)

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
    # the serial port and read the results.
    with open(save_file_path, mode) as save_file:
        for bin_file in bin_files:
            
            # if bin_file was already tested, skip it
            if args.cont:
                if os.path.basename(bin_file) in tested_files:
                    continue
            
            while(True):
                with ConnectHelper.session_with_chosen_probe() as session:
                    try:                        
                        print("Testing: {0}".format(bin_file))

                        board = session.board
                        target = board.target
                        flash = target.memory_map.get_boot_memory()
                        
                        # Load firmware into device.
                        FileProgrammer(session).program(bin_file)
                        
                        # Reset, run.
                        target.reset_and_halt()
                        target.resume()
                    
                        r, results, test, s1, s2, s3 = read_results(ser, args.taskcnt)
                        
                        if results:
                            # save results in save_file file
                            for r in results:
                                save_file.write("{1}\t{0}\t{2}\t{3}\t{4}".format(os.path.basename(bin_file), r.rstrip('\n\r'), slack[s1], slack_method[s2], slack_k[s3]))
                            ok_counter = ok_counter + 1
                        else:
                            # increment the fail counters
                            errors[0] = errors[0] + 1
                            errors[r] = errors[r] + 1
                            deadline_miss.append(os.path.basename(bin_file))
                        break
                    except ValueError as err:
                        # something is wrong...
                        print("ValueError: {0}".format(str(err)), file=sys.stderr)
                    except IndexError as err:
                        print("IndexError: {0}".format(str(err)), file=sys.stderr)
                    except (IOError, os.error) as err:
                        print("IOerror: {0}".format(str(err)), file=sys.stderr)
                        break
                    except (TransferError, DAPAccessIntf) as err:
                        print("!")
                        
            sleep(1)

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