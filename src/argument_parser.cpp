#include "argument_parser.h"

#include <iostream>
#include <vector>

using std::string;
using std::vector;

cxxopts::ParseResult ParseArguments(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], " - example command line options");
    options
      .positional_help("[optional args]")
      .show_positional_help();

    options
        .add_options()
        ("workflow", "Workflow description", cxxopts::value<string>())
        ("vm_list", "List of virtual machines available", cxxopts::value<string>())
        ("p, platform", "Platform description file", cxxopts::value<string>(), "PLATFORM")
        ("s, save_platform", "Save platform description file", cxxopts::value<bool>()->default_value("false"), "SAVE PLATFORM")
        ("d, deadline", "Workflow deadline", cxxopts::value<double>(), "DEADLINE")
        ("h, help", "Show help");

    vector<string> positionalArguments = {"workflow", "vm_list"};

    options.parse_positional(positionalArguments.begin(), positionalArguments.end());
    options.positional_help("[workflow] [vm_list] [optional_args]");

    cxxopts::ParseResult result = options.parse(argc, argv);

    if (result.count("help") || result.arguments().size() < positionalArguments.size()) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    return result;
}