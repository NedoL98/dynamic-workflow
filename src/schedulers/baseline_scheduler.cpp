#include "baseline_scheduler.h"

using std::optional;
using std::map;
using std::max;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(baseline_scheduler, "Baseline scheduler log");

BaselineScheduler::Actions BaselineScheduler::PrepareForRun(View::Viewer& v) {
    viewer.reset(&v);

    return MakeSchedule();
}

BaselineScheduler::Actions BaselineScheduler::MakeSchedule() const {
    BaselineScheduler::Actions actions;
    Schedule s;

    vector<View::Task> tasksOrder = viewer->MakeTasksOrder();
    vector<VMDescription> availableVMs;
    vector<double> vmEndTime; 
    vector<double> endTime(viewer->WorkflowSize(), -1);

    double makespan = 0;
    double cost = 0;

    for (const View::Task& task: tasksOrder) {
        double taskBeginTime = 0;
        for (int dependencyId: task.GetDependencies()) {
            xbt_assert(endTime[dependencyId] != -1, "Can't process next task, task order is inconsistent!");
            taskBeginTime = max(taskBeginTime, endTime[dependencyId]);
        }

        optional<int> bestVMId;
        for (int vmId = 0; vmId < static_cast<int>(availableVMs.size()); ++vmId) {
            if (vmEndTime[vmId] <= taskBeginTime && (!bestVMId.has_value() || availableVMs[bestVMId.value()] > availableVMs[vmId])) {
                bestVMId = vmId;
            }
        }

        if (!bestVMId.has_value()) {
            VMDescription bestVMDescr = GetBestVM(task);

            availableVMs.push_back(bestVMDescr);
            vmEndTime.push_back(-1);
            bestVMId = availableVMs.size() - 1;

            actions.push_back(std::make_shared<BuyVMAction>(bestVMDescr, bestVMId.value()));
        }
        s.AddItem(bestVMId.value(), ScheduleItem(task.GetId()));

        double taskEndTime = taskBeginTime + task.GetExecutionTime(availableVMs[bestVMId.value()]);
        vmEndTime[bestVMId.value()] = taskEndTime;
        endTime[task.GetId()] = taskEndTime;

        makespan = max(makespan, taskEndTime);
        cost += task.GetExecutionCost(availableVMs[bestVMId.value()]);
    }

    actions.push_back(std::make_shared<ResetScheduleAction>(s));

    XBT_INFO("Schedule created!");
    XBT_INFO("Makespan: %f", makespan);
    XBT_INFO("Cost: %f", cost);

    return actions;
}

VMDescription BaselineScheduler::GetBestVM(const View::Task& task) const {
    optional<VMDescription> bestVM;
    for (const VMDescription& vmDescr: viewer->GetVMList()) {
        // TODO: reconsider criterion for best vm
        if (task.CanBeExecuted(vmDescr) && (!bestVM.has_value() || bestVM > vmDescr)) {
            bestVM = vmDescr;
        }
    }
    xbt_assert(bestVM.has_value(), "No suitable VM for task %d found!", task.GetId());
    return bestVM.value();
}
