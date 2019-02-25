#include "prototypes/simulator.h"
#include "view/viewer.h"

#include <memory>

void CloudSimulator::Run(double timeout) {
    View::Viewer v(*this);
    std::vector<std::shared_ptr<AbstractAction>> actions = Scheduler->PrepareForRun(&v);
    for (auto a : actions) {
        ProcessAction(a);
    }
}
