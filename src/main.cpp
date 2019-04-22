/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "argument_parser.h"
#include "platform_generator.h"
#include "prototypes/simulator.h"
#include "schedulers/genetic_scheduler.h"
#include "schedulers/mao_scheduler.h"
#include "vm_list.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(main, "Main log");

using std::string;

int main(int argc, char* argv[]) {
    cxxopts::ParseResult parseResult = ParseArguments(argc, argv);

    // Make suitable platform for workflow
    string platformPath;
    if (!parseResult.count("platform")) {
        platformPath = GeneratePlatform(parseResult["workflow"].as<string>(), parseResult["vm_list"].as<string>());
    } else {
        platformPath = parseResult["platform"].as<string>();
    }

    // Load tasks graph description
    // MaoScheduler scheduler = MaoScheduler(parseResult["workflow"].as<string>(), parseResult["vm_list"].as<string>());

    simgrid::s4u::Engine e(&argc, argv);

    MaoScheduler dummy;

    CloudSimulator cloudSim(platformPath,
                            parseResult["workflow"].as<string>(),
                            parseResult["vm_list"].as<string>(),
                            &dummy,
                            parseResult);

    cloudSim.Run();

    XBT_INFO("Simulation is over");

    if (!parseResult.count("platform") && parseResult["save_platform"].as<bool>() == false) {
        remove(platformPath.c_str());
    }
}
