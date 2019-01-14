from xml.etree import ElementTree
import ruamel.yaml
import os

yaml = ruamel.yaml.YAML()

def MakeYaml(xmlFileName):
    tree = ElementTree.parse(xmlFileName)

    outputYaml = {}
    outputYaml["name"] = xmlFileName
    outputYaml["tasks"] = []

    taskOutputs = {}
    fileSize = {}
    workflowInputs = set()
    inputs = set()

    def GetPreYaml(node):
        if "id" in node.attrib:
            task = {}
            nodeId = node.attrib["id"]
            task["name"] = nodeId
            task["inputs"] = []
            task["outputs"] = []
            task["size"] = float(node.attrib["runtime"])
            for child in node:
                filename = child.attrib["file"].replace(".", "_")
                if child.attrib["link"] == "input":
                    task["inputs"].append({"name": filename})
                    inputs.add(filename)
                elif child.attrib["link"] == "output": 
                    taskOutputs[filename] = nodeId
                    task["outputs"].append({"name": filename, "size": int(child.attrib["size"])})
                fileSize[filename] = int(child.attrib["size"])
            outputYaml["tasks"].append(task)
        else:
            for child in node:
                GetPreYaml(child)

    def GetSources():
        for task in outputYaml["tasks"]:
            for input in task["inputs"]:
                if input["name"] not in taskOutputs:
                    input["source"] = input["name"]
                    workflowInputs.add(input["name"])
                else:
                    input["source"] = taskOutputs[input["name"]] + "." + input["name"]

    def InsertInputs():
        outputYaml["inputs"] = []
        for inputName in workflowInputs:
            outputYaml["inputs"].append({"name": inputName, "size": fileSize[inputName]})

    def InsertOutputs():
        outputYaml["outputs"] = []
        for task in outputYaml["tasks"]:
            for output in task["outputs"]:
                outputName = output["name"]
                if outputName not in inputs:
                    source = taskOutputs[outputName] + "." + outputName
                    outputYaml["outputs"].append({"name": outputName, "source": source})

    def GetYaml(root):
        GetPreYaml(root)
        
        # Set a source for each input file
        GetSources()
        
        # Insert all workflow inputs
        InsertInputs()
        
        # Insert all workflow outputs
        InsertOutputs()

    GetYaml(tree.getroot())

    with open(xmlFileName.replace("xml", "yml"), 'w') as outfile:
        yaml.indent(offset = 2, sequence = 4)
        yaml.dump(outputYaml, outfile)

for dirName, subDirList, fileList in os.walk("."):
    for fileName in fileList:
        if fileName.find(".xml") != -1:
            MakeYaml(fileName)