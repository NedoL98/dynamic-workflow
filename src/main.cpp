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
#include "schedulers/naive_scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(main, "Main log");

int main(int argc, char* argv[]) {
    cxxopts::ParseResult parseResult = ParseArguments(argc, argv);

    // Load tasks graph description
    NaiveScheduler scheduler = NaiveScheduler(parseResult["workflow"].as<string>());

    // Make suitable platform for workflow
    string platformPath;
    if (!parseResult.count("platform")) {
        platformPath = GeneratePlatform(scheduler);
    } else {
        platformPath = parseResult["platform"].as<string>();
    }

    simgrid::s4u::Engine e(&argc, argv);

    // Load the platform description
    e.load_platform(platformPath);
    XBT_INFO("Platform file loaded");

    // Transform human-readable platform description
    TransformHostsProps();

    // Make scheduler actor
    simgrid::s4u::Actor::create("scheduler", e.get_all_hosts()[0], scheduler);

    XBT_INFO("Starting simulation");
    // Run the simulation
    e.run();

    XBT_INFO("Simulation is over");

    if (!parseResult.count("platform") && parseResult["save_platform"].as<bool>() == false) {
        remove(platformPath.c_str());
    }
}
