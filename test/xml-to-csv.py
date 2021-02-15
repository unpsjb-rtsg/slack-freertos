import os
import sys
import glob
import tempfile
import subprocess
import xml.etree.cElementTree as et
import math
from typing import TextIO
from argparse import ArgumentParser, FileType
from functools import reduce


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


def get_args():
    """ Command line arguments """
    parser = ArgumentParser(description="Evaluate RTS in XML file.")
    parser.add_argument("--rts", type=str, help="RTS inside file(s) to test.")
    parser.add_argument("--xml", type=FileType('r'), help="XML files with RTS to test.", nargs="+")
    #parser.add_argument("csv", type=FileType('w'), help="CSV file to create.")
    return parser.parse_args()


def main():
    if not len(sys.argv) > 1:
        print("No arguments!", file=sys.stderr)
        exit()

    args = get_args()
    rts_list = mixrange(args.rts)

    for xml_file in args.xml:
        print("File: {0}".format(xml_file.name))
        for rts in get_from_xml(xml_file, rts_list):
            for task in rts["tasks"]:
                print("{0:} {1:} {2:} {3:} {4:}".format(rts["id"], task["nro"], task["C"], task["T"], task["D"]))


if __name__ == '__main__':
    main()
