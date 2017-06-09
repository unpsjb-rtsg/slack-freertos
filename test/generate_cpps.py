import os
import sys
import glob
import tempfile
import subprocess
import collections
from argparse import ArgumentParser

def test_rts(id, xmlfile):
    import math
    import xml.etree.cElementTree as et

    def get_rts_from_element(elem):
        """ extrae el str de elem """

        def get_int(string):
            """ convierte string a int """
            try:
                return int(string)
            except ValueError:
                return int(float(string))

        rts_id, rts = 0, []
        if elem.tag == 'S':
            rts_id = get_int(elem.get("count"))
            for t in elem.iter("i"):
                task = t.attrib
                for k, v in task.items():
                    task[k] = get_int(v)
                rts.append(task)
        
        return rts_id, rts

    rts_to_test_id = id
    rts_to_test = []

    context = et.iterparse(xmlfile, events=('start', 'end', ))
    context = iter(context)
    event, root = next(context) #context.next()
    for event, elem in context:
        rts_id, rts_to_test = get_rts_from_element(elem)
        if rts_to_test_id == rts_id and rts_to_test and event == "end":
            break
        root.clear()
    del context
    
    result = joseph_wcrt(rts_to_test)
    
    return result[0]
    

def joseph_wcrt(rts):
    """ Verify schedulability """
    import math

    wcrt = [0] * len(rts)
    schedulable = True
    wcrt[0] = rts[0]["C"]
    for i, task in enumerate(rts[1:], 1):
        r = 1
        c, t, d = task["C"], task["T"], task["D"]        
        while schedulable:
            w = 0
            for taskp in rts[:i]:
                cp, tp = taskp["C"], taskp["T"]
                w += math.ceil(r / tp) * cp                
            w = c + w
            if r == w:            
                break
            r = w
            if r > d:
                schedulable = False
        wcrt[i] = r
        if not schedulable: break
    return [schedulable, wcrt]

    
def get_args():
    """ Command line arguments """
    freertos_choices = ["v8.1.2", "v9.0.0"]
    slack_choices = ["ss","k"]
    slackmethod_choices = ["fixed", "davis"]
    test_choices = ['cycles-cs', 'ceils', 'cycles-ss', 'loops']

    parser = ArgumentParser(description="Generate CPP files, each of them implementing a set of real-time tasks using the FreeRTOS real-time operating system.")

    parser.add_argument("count", help="Number of CPP files to generate for each XML file. Each CPP file will implement a task-set of several periodic tasks.", type=int)
    parser.add_argument("xml_files", help="XML file with real-time task-sets from which generate CPPs files.", nargs="+", metavar="xml")
    
    source_group = parser.add_argument_group('Source code files', 'This options control how the CPP files are generated from a task-set.')
    source_group.add_argument("--start", help="Start generating CPPs starting from this offset. Defaults to %(default)s.", type=int, default=1)
    source_group.add_argument("--template", help="Template file used by COG to generate the CPP files. Defaults to %(default)s.", type=str, default="main.cpp")
    source_group.add_argument("--srcpath", help="Save path where the generated CPP files will be stored.", type=str)
    source_group.add_argument("--ignore", help="A list of RTS to ignore in the XML files.", nargs='+', type=int, default=[])

    schedtest_group = parser.add_argument_group('Scheduling evaluation', 'This options control if a scheduling analysis is to be performed before a CPP file is generated from a task-set.')
    schedtest_group.add_argument("--testsched", help="Check the schedulability of each RTS before generating a CPP file. Defaults to %(default)s.", action="store_true", default=True)
    schedtest_group.add_argument("--limit", help="Maximum amount of RTS to test for schedulability.", type=int, default=None)

    bin_group = parser.add_argument_group('Compilation options', 'This options control how the CPP files are compiled.')
    bin_group.add_argument("--bins", help="Compile the CPP files.", action="store_true", default=False)
    bin_group.add_argument("--freertos", help="FreeRTOS version to use. Valid values are " + ', '.join(freertos_choices) + ". Defaults to %(default)s.", choices=freertos_choices, default=freertos_choices[0])
    bin_group.add_argument("--taskcnt", help="Number of tasks in each RTS. Defaults to %(default)s.", type=int, default=10)
    bin_group.add_argument("--releasecnt", help="Number of task releases to evaluate. Defaults to %(default)s.", type=int, default=10)
    bin_group.add_argument("--slack", help="Use Slack Stealing.", action="store_const", const=1, default=0)
    bin_group.add_argument("--slackcalc", help="When to calculate slack. Valid values are " + ', '.join(slack_choices) + ". Defaults to %(default)s.", choices=slack_choices, default=slack_choices[0])
    bin_group.add_argument("--slackmethod", help="Slack Stealing method to test. Valid values are " + ', '.join(slackmethod_choices) + ". Defaults to %(default)s.", choices=slackmethod_choices, default=slackmethod_choices[0])
    bin_group.add_argument("--debug", help="Set the GCC debug option (-G).", action="store_const", const=1, default=0)
    bin_group.add_argument("--test", help="Test to perform. Available tests are " + ', '.join(test_choices) + ". Defaults to %(default)s.", choices=test_choices, default=test_choices[0] )

    return parser.parse_args()


def main():
    slack_calc = collections.OrderedDict({ 'ss': 0, 'k': 1 })
    slack_methods = { 'fixed': 0, 'davis': 1 }
    test_names = { 'cycles-cs': 1, 'ceils': 2, 'cycles-ss': 3, 'loops': 4}
    
    args = get_args()
    
    if not os.path.isdir(args.srcpath):
        print("{0}: path not found.".format(args.srcpath), file=sys.stderr)
        sys.exit(1)

    return_code = 0

    xml_file_list = []
    for xml_file in args.xml_files:
        if not os.path.isfile(xml_file):
            print("warning: {0} file not found.".format(xml_file), file=sys.stderr)
        else:
            xml_file_list.append(xml_file)
    
    if not xml_file_list:
        print("error: no files found.", file=sys.stderr)
        sys.exit(1)
        
    # remove previous generated bin files
    subprocess.call("make --no-print-directory -C {0} clean".format(args.srcpath), shell=True, stdout=None, stderr=None)
	
    if xml_file_list:
        # remove previous generated source code files
        print("Remove previous source code files...")
        for src_file in glob.glob("{0}/*.cpp".format(args.srcpath)):
            os.remove(src_file)
    
        tmp_file = tempfile.NamedTemporaryFile(delete=False, mode="r+b")
		
        print("Generating CPPs files...")
        
        for xml_file in xml_file_list:
            xml_file_name = os.path.basename(xml_file)
    
            # xml file name
            bin_file_name = os.path.splitext(xml_file_name)[0]

            if args.testsched:
                rts_found = 0
                limit = 0
                rts_id = args.start
                while rts_found < args.count:
                    if test_rts(rts_id, xml_file):
                        if rts_id not in args.ignore:
                            cpp_file = os.path.join(args.srcpath, "{0}_{1:05}.cpp".format(bin_file_name, rts_id))
                            cog_str = "{0} -D RTS_TO_TEST={1} -D RTS_FILE={2} -d -o {3}\n".format(args.template, rts_id, xml_file, cpp_file)
                            tmp_file.write(bytes(cog_str, "UTF-8"))
                        rts_found = rts_found + 1
                    else:
                        limit = limit + 1
                    
                    rts_id = rts_id + 1
    
                    if args.limit and limit > args.limit:
                        break
            else:
                for x in range(args.start, args.start + args.count):
                    cpp_file = os.path.join(args.srcpath, "{0}_{1:05}.cpp".format(bin_file_name, x))
                    cog_str = "{0} -D RTS_TO_TEST={1} -D RTS_FILE={2} -d -o {3}\n".format(args.template, x, xml_file, cpp_file)
                    tmp_file.write(bytes(cog_str))
    
        # from the python docs: "Whether the name can be used to open the file a
        # second time, while the named temporary file is still open, varies across
        # platforms (it can be so used on Unix; it cannot on Windows NT or later)."
        tmp_file.close()
    
        # cog para que procese los comandos que contiene el archivo temporal
        subprocess.call("python -m cogapp @{0}".format(tmp_file.name), shell=True)
    
        # ahora si eliminamos el archivo temporal
        os.remove(tmp_file.name)
    
    if(args.bins):
        # generate bins        
        make_string = [ "make --no-print-directory -C {0}".format(args.srcpath), 
                        "MAX_PRIO=1",
                        "DEBUG={0}".format(args.debug),
                        "TASK_COUNT_PARAM={0}".format(args.taskcnt),
                        "RELEASE_COUNT_PARAM={0}".format(args.releasecnt),
                        "SLACK={0}".format(args.slack),
                        "SLACK_K={0}".format(slack_calc[args.slackcalc]),
                        "SLACK_METHOD={0}".format(slack_methods[args.slackmethod]),
                        "TEST_PATH={0}".format(args.srcpath),
                        "KERNEL_TEST={0}".format(test_names[args.test]),
                        "FREERTOS_KERNEL_VERSION_NUMBER={0}".format(args.freertos),
                        "FREERTOS_KERNEL_VERSION_NUMBER_MAJOR={0}".format(int(args.freertos[1])) ]
        returncode = subprocess.call(" ".join(make_string), shell=True, stdout=None, stderr=None)
    
    if returncode != 0:
        print("Something went wrong!")


if __name__ == '__main__':
    main()
