#pragma once
#include "platform/host.h"


enum EState {
    NotScheduled,
    Scheduled,
    Ready,
    Running,
    Done
};

struct Status {
    Host* Assignment;
    EState State;
};

