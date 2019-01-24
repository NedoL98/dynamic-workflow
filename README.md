# Dynamic workflow manager

Requirements:

* [yaml-cpp](https://github.com/jbeder/yaml-cpp) version 1.1 or above.
* [SimGrid](http://simgrid.gforge.inria.fr) version 3.2 or above.

Project is using [cxxopts](https://github.com/jarro2783/cxxopts) library for parsing command-line parameters. There is no need to download the library, it is explicitly included in this project. 

Tools:

* `xml_yaml_parser.py` -- tool for converting workflow files from old .dax format to pretty .yaml. Depends on package `python3-ruamel.yaml` >= 0.15, install it through pip: `pip install ruamel.yaml`.

How to launch:

```
cmake CMakeLists.txt 
make
build/dynamic-workflow small_platform.xml master_workers.xml
```
