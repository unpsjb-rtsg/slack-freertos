import glob
import os
import tempfile
import subprocess
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
    event, root = context.__next__()
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
    parser = ArgumentParser(description="Generate CPPs")
    parser.add_argument("--start", help="RTS to start from", type=int, default=1)
    parser.add_argument("--count", help="Number of RTS to test", type=int)
    parser.add_argument("--limit", help="Number of RTS to test for schedulability", type=int, default=1000)
    parser.add_argument("--template", help="COG template file", type=str)
    parser.add_argument("--srcpath", help="src directory", type=str)
    parser.add_argument("--xmlpath", help="xml directory", type=str)
    parser.add_argument("--ignore", nargs='+', type=int, help="rts ignore list", default=[])
    parser.add_argument("--cpps", help="Generate cpp files", action="store_true", default=False)
    parser.add_argument("--bins", help="Generate bin files", action="store_true")
    parser.add_argument("--freertos", help="Build FreeRTOS", action="store_true")
    parser.add_argument("--taskcnt", help="Number of tasks", type=int, default=10)
    parser.add_argument("--releasecnt", help="Number of instances to test", type=int, default=10)
    parser.add_argument("--testsched", help="Check rts schedulability", action="store_true")
    parser.add_argument("--slack", help="Use Slack Stealing.", action="store_const", const=1, default=0)
    parser.add_argument("--slackcalc", help="When to calculate slack.", choices=["ss","k"], default="ss")
    parser.add_argument("--slackmethod", help="Slack Stealing method to test.", choices=["fixed", "davis"], default="fixed")
    parser.add_argument("--debug", help="Set debug -G option.", action="store_const", const=1, default=0)
    return parser.parse_args()


def main():
    args = get_args()
    
    slack_calc = { 'ss':0, 'k':1 }
    slack_methods = { 'fixed':0, 'davis':1 } 
    
    # make freertos library
#    if args.freertos:
#        print("Make FreeRTOS library...")
#        subprocess.call("make -C {0}/FreeRTOS/ clean".format(args.srcpath), shell=True)
#        subprocess.call("make -C {0}/FreeRTOS/ SLACK={1} SLACK_K={2} SLACK_METHOD={3} DEBUG={4} TASK_CNT={5} RELEASE_CNT={6}".format(args.srcpath, args.slack, slack_calc[args.slackcalc], slack_methods[args.slackmethod], args.debug, args.taskcnt, args.releasecnt), shell=True)
    
    xml_file_list = glob.glob("{0}/*.xml".format(args.xmlpath));
        
    # remove previous generated bin files
    print("Remove previous binary files...")
    subprocess.call("make -C {0} clean".format(args.srcpath), shell=True, stdout=None, stderr=None)
	
    if args.cpps:
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
                        print("str {0} in {1} not schedulable.".format(rts_id, xml_file))
                        limit = limit + 1
                    rts_id = rts_id + 1
    
                    if limit > args.limit:
                        break             
            else:
                for x in range(args.start, args.start + args.count):
                    cpp_file = os.path.join(args.srcpath, "{0}_{1:05}.cpp".format(bin_file_name, x))
                    cog_str = "{0} -D RTS_TO_TEST={1} -D RTS_FILE={2} -d -o {3}\n".format(args.template, x, xml_file, cpp_file)
                    #tmp_file.write(bytes(cog_str, "UTF-8"))
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
        print("Generate binaries...")
        subprocess.call("make -C {0} BATCH_TEST=1 DEBUG={1} TASK_CNT={2} RELEASE_CNT={3} SLACK={4} SLACK_K={5} SLACK_METHOD={6} TEST_PATH={0}".format(args.srcpath, args.debug, args.taskcnt, args.releasecnt, args.slack, slack_calc[args.slackcalc], slack_methods[args.slackmethod]), shell=True, stdout=None, stderr=None)
    
    print("Done!")


if __name__ == '__main__':
    main()
