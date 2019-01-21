/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "schedulers/naive_scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(main, "Main log");

int main(int argc, char* argv[]) {
    xbt_assert(argc > 2, "Usage: %s platform_file tasks_graph_file\n", argv[0]);
    simgrid::s4u::Engine e(&argc, argv);

    // Load the platform description
    e.load_platform(argv[1]);
    XBT_INFO("Platform file loaded");

    // Transform human-readable platform description
    TransformHostsProps();

    // Load tasks graph description
    auto scheduler = NaiveScheduler(argc, argv);

    // Make scheduler actor
    simgrid::s4u::Actor::create("scheduler", simgrid::s4u::Host::by_name("Alpha"), scheduler);

    XBT_INFO("Starting simulation");
    // Run the simulation
    e.run();

    XBT_INFO("Simulation is over");
}
