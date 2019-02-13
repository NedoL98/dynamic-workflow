#include "prototypes/simulator.h"
#include "viewer.h"

#include <memory>

void CloudSimulator::Run(double timeout) {
    Viewer v(*this);
    std::vector<std::shared_ptr<AbstractAction>> actions = Scheduler->PrepareForRun(&v);
    for (auto a : actions) {
        ProcessAction(a);
    }
}
