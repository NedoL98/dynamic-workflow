from xml.etree import ElementTree
from ruamel.yaml.comments import CommentedMap, CommentedSet
import ruamel.yaml
import os
import argparse

yaml = ruamel.yaml.YAML()

def GetPreYaml(node, task_inputs, task_outputs, file_size, output_yaml):
    if "id" in node.attrib:
        task = CommentedMap()
        nodeId = node.attrib["id"]
        task["name"] = nodeId
        task["inputs"] = []
        task["outputs"] = []
        task["size"] = node.attrib["runtime"] + "GF"
        for child in node:
            filename = child.attrib["file"]
            if child.attrib["link"] == "input":
                task["inputs"].append({"name": filename})
                task_inputs.add(filename)
            elif child.attrib["link"] == "output": 
                task_outputs[filename] = nodeId
                task["outputs"].append({"name": filename, "size": int(child.attrib["size"])})
            file_size[filename] = int(child.attrib["size"])
        output_yaml["tasks"].append(task)
    else:
        for child in node:
            GetPreYaml(child, task_inputs, task_outputs, file_size, output_yaml)

def GetTaskSources(workflow_inputs, task_outputs, output_yaml):
    for task in output_yaml["tasks"]:
        for input in task["inputs"]:
            if input["name"] not in task_outputs:
                workflow_inputs.add(input["name"])
            else:
                input["source"] = task_outputs[input["name"]]

def InsertWorkflowInputs(workflow_inputs, file_size, output_yaml):
    output_yaml.insert(1, "inputs", [])
    for input_name in workflow_inputs:
        output_yaml["inputs"].append({"name": input_name, "size": file_size[input_name]})

def InsertWorkflowOutputs(workflow_inputs, task_outputs, output_yaml):
    output_yaml.insert(2, "outputs", [])
    for task in output_yaml["tasks"]:
        for output in task["outputs"]:
            output_name = output["name"]
            if output_name not in workflow_inputs:
                source = task_outputs[output_name]
                output_yaml["outputs"].append({"name": output_name, "source": source})

def MakeYaml(xml_filename, output_dir):
    tree = ElementTree.parse(xml_filename)

    output_yaml = CommentedMap()
    output_yaml["name"] = xml_filename.split('/')[-1].replace("xml", "yml")
    output_yaml["tasks"] = []

    task_inputs = CommentedSet()
    task_outputs = CommentedMap()
    file_size = CommentedMap()

    workflow_inputs = CommentedSet()

    GetPreYaml(tree.getroot(), task_inputs, task_outputs, file_size, output_yaml)

    GetTaskSources(workflow_inputs, task_outputs, output_yaml)

    InsertWorkflowInputs(workflow_inputs, file_size, output_yaml)

    InsertWorkflowOutputs(workflow_inputs, task_outputs, output_yaml)

    with open(output_dir + output_yaml["name"], 'w+') as outfile:
        yaml.indent(offset = 2, sequence = 4)
        yaml.dump(output_yaml, outfile)

parser = argparse.ArgumentParser(description = "xml to yml converter")
parser.add_argument("output_dir", type = str, help = 'Path to output directory')
parser.add_argument("filepaths", nargs = '+', help = 'Path to xml platform descriptions that should be converted')
args = parser.parse_args()

if args.output_dir is not None:
    if not os.path.isdir(args.output_dir):
        print(args.output_dir + ' is not a directory!')
        exit(0)
    if not args.output_dir.endswith('/'):
        args.output_dir += '/'

for filepath in args.filepaths:
    if not os.path.isfile(filepath):
        print(filepath + ' is not a file!')
    else:
        try:
            MakeYaml(filepath, args.output_dir)
        except:
            print('Error while parsing ' + filepath)