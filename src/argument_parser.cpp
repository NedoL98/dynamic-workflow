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
        ("p, platform", "Platform description file", cxxopts::value<string>(), "PLATFORM")
        ("s, save_platform", "Save platform description file", cxxopts::value<bool>()->default_value("false"), "SAVE PLATFORM")
        ("h, help", "Show help");

    vector<string> positionalArguments = {"workflow"};

    options.parse_positional(positionalArguments.begin(), positionalArguments.end());

    cxxopts::ParseResult result = options.parse(argc, argv);

    if (result.count("help") || result.arguments().size() == 0) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    return result;
}