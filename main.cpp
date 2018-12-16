/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(main, "Logging main");

int main(int argc, char* argv[]) {
    xbt_assert(argc > 3, "Usage: %s platform_file deployment_file tasks_graph_file\n", argv[0]);
    simgrid::s4u::Engine e(&argc, argv);

    TasksGraph tasksGraph(argv[3]);
    tasksGraph.PrintGraph();

    // Register scheduler
    e.register_actor<Scheduler>("scheduler");
    XBT_INFO("Scheduler registered");

    // Load the platform description and then deploy the application
    e.load_platform(argv[1]);
    e.load_deployment(argv[2]);
    XBT_INFO("Platform and deployment files loaded");

    // Although we can't explicitly pass the amount of memory each host has
    // we can declare it somewhere else and then parse it in the following fashion
    /*
    std::vector<simgrid::s4u::Host*> hosts = e.get_all_hosts();
    int i = 1;
    for (simgrid::s4u::Host* host : hosts) {
        host->set_property("ram", std::to_string(i) + "Gb");
        ++i;
    }
    ...
    ...
    ...
    int hostRam = stoi(host->get_property("ram"));
    */


    XBT_INFO("Starting simulation");
    // Run the simulation
    e.run();

    XBT_INFO("Simulation is over");
}