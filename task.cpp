#include "task.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(task, "Task log");

Task::Task(long long flops, int cores, int ram) 
    : Flops(flops)
    , Cores(cores)
    , Ram(ram)
    {}

long long Task::GetFlops() {
    return Flops;
}

int Task::GetCores() {
    return Cores;
}

int Task::GetRam() {
    return Ram;
}

void Task::DoExecute(long long flops) {
    double timeStart = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::this_actor::execute(flops);
    double timeFinish = simgrid::s4u::Engine::get_clock();

    XBT_INFO("%s:%s task executed %g", simgrid::s4u::this_actor::get_host()->get_cname(),
            simgrid::s4u::this_actor::get_cname(), timeFinish - timeStart);
}

void Task::Execute(simgrid::s4u::Host* host) {
    simgrid::s4u::Actor::create("compute", host, DoExecute, Flops);
}