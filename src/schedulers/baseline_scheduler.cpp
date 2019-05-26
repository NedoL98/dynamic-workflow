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
    vector<View::Task> tasksOrder = viewer->MakeTasksOrder();
    vector<double> vmEndTime; 
    vector<double> taskEndTime(viewer->WorkflowSize(), 0);
    vector<int> taskIdToVMId(viewer->WorkflowSize(), -1);
    vector<int> taskIdToVMPosition(viewer->WorkflowSize(), -1);

    vector<VMData> availableVMs;

    double avgLag = static_cast<double>(viewer->GetVMList().GetMinLag() + viewer->GetVMList().GetMaxLag()) / 2;

    for (const View::Task& task: tasksOrder) {
        double taskBeginTime = 0;
        int taskId = task.GetId();

        for (int dependencyId: task.GetDependencies()) {
            xbt_assert(taskEndTime[dependencyId] != -1, "Can't process next task, task order is inconsistent!");
            taskBeginTime = max(taskBeginTime, taskEndTime[dependencyId]);
        }

        optional<int> bestVMId;
        for (int vmId = 0; vmId < static_cast<int>(availableVMs.size()); ++vmId) {
            if (vmEndTime[vmId] <= taskBeginTime &&
                task.CanBeExecuted(availableVMs[vmId].VMDescr) &&
                (!bestVMId.has_value() || availableVMs[bestVMId.value()].VMDescr > availableVMs[vmId].VMDescr)) {
                bestVMId = vmId;
            }
        }

        if (!bestVMId.has_value()) {
            // Allocate new VM if there is no suitable VM available
            taskBeginTime += avgLag;
            VMDescription bestVMDescr = GetBestVM(task);

            availableVMs.push_back({bestVMDescr, {}});
            vmEndTime.push_back(-1);
            bestVMId = availableVMs.size() - 1;
        }
        taskIdToVMId[task.GetId()] = bestVMId.value();
        taskIdToVMPosition[taskId] = availableVMs[bestVMId.value()].TasksQueue.size();
        
        availableVMs[bestVMId.value()].TasksQueue.push_back(taskId);

        double currentTaskEndTime = taskBeginTime + task.GetExecutionTime(availableVMs[bestVMId.value()].VMDescr);
        vmEndTime[bestVMId.value()] = currentTaskEndTime;
        taskEndTime[taskId] = currentTaskEndTime;
    }

    double makespan = 0;
    double cost = 0;
    DoCalculateMakespanAndCost(availableVMs, taskIdToVMId, taskIdToVMPosition, makespan, cost);
    while (makespan > viewer->GetDeadline()) {
        XBT_INFO("Current makespan: %f", makespan);
        XBT_INFO("Current cost: %f", cost);
        UpgradeRandomVM(availableVMs);
        DoCalculateMakespanAndCost(availableVMs, taskIdToVMId, taskIdToVMPosition, makespan, cost);
    }

    XBT_INFO("Schedule created!");
    XBT_INFO("Makespan: %f", makespan);
    XBT_INFO("Cost: %f", cost);
    XBT_INFO("Using %d virtual machines", availableVMs.size());

    return MakeActions(availableVMs);
}

void BaselineScheduler::DoCalculateMakespanAndCost(const vector<VMData>& availableVMs, 
                                                   const vector<int>& taskIdToVMId,
                                                   const vector<int>& taskIdToVMPosition,
                                                   double& makespan,
                                                   double& cost) const {
    makespan = 0;

    vector<double> vmStartTime(availableVMs.size());
    vector<double> vmEndTime(availableVMs.size(), 0);
    vector<double> taskEndTime(viewer->WorkflowSize());

    double avgLag = static_cast<double>(viewer->GetVMList().GetMinLag() + viewer->GetVMList().GetMaxLag()) / 2;

    // FIXME if MakeTasksOrder is inconsistent
    vector<View::Task> tasksOrder = viewer->MakeTasksOrder();
    for (const View::Task& task: tasksOrder) {
        int taskId = task.GetId();
        int vmId = taskIdToVMId[taskId];

        double taskBeginTime = 0;
        for (int dependencyId: task.GetDependencies()) {
            xbt_assert(taskEndTime[dependencyId] != -1, "Can't process next task, task order is inconsistent!");
            taskBeginTime = max(taskBeginTime, taskEndTime[dependencyId]);
        }
        if (taskIdToVMPosition[taskId] != 0) {
            int prevTaskId = availableVMs[vmId].TasksQueue[taskIdToVMPosition[taskId] - 1];
            xbt_assert(taskEndTime[prevTaskId] != -1, "Can't process next task, task order is inconsistent!");
            taskBeginTime = max(taskBeginTime, taskEndTime[prevTaskId]);
        } else {
            taskBeginTime += avgLag;
            // we don't pay for VM acquisition time
            vmStartTime[vmId] = taskBeginTime;
        }

        taskEndTime[taskId] = taskBeginTime + task.GetExecutionTime(availableVMs[vmId].VMDescr);
        xbt_assert(vmEndTime[vmId] < taskEndTime[taskId], "Something went wrong, task order is inconsistent!");
        vmEndTime[vmId] = taskEndTime[taskId];
        makespan = max(makespan, taskEndTime[taskId]);
    }

    cost = 0;
    for (int i = 0; i < static_cast<int>(availableVMs.size()); ++i) {
        cost += (vmEndTime[i] - vmStartTime[i]) * availableVMs[i].VMDescr.GetPrice();
    }
}

bool BaselineScheduler::UpgradeRandomVM(vector<VMData>& availableVMs) const {
    vector<int> upgradableVMs;
    for (int vmId = 0; vmId < static_cast<int>(availableVMs.size()); ++vmId) {
        for (VMDescription vmDescr: viewer->GetVMList()) {
            bool suitableVM = true;
            for (int taskId: availableVMs[vmId].TasksQueue) {
                if (!viewer->GetTaskById(taskId).CanBeExecuted(availableVMs[vmId].VMDescr)) {
                    suitableVM = false;
                    break;
                }
            }
            if (suitableVM && vmDescr > availableVMs[vmId].VMDescr) {
                upgradableVMs.push_back(vmId);
                break;
            }
        }
    }

    if (!upgradableVMs.empty()) {
        int randomVMId = rand() % upgradableVMs.size();
        optional<VMDescription> newVMType; 
        for (VMDescription vmDescr: viewer->GetVMList()) {
            bool suitableVM = true;
            for (int taskId: availableVMs[randomVMId].TasksQueue) {
                if (!viewer->GetTaskById(taskId).CanBeExecuted(vmDescr)) {
                    suitableVM = false;
                    break;
                }
            }
            if (suitableVM && 
                vmDescr > availableVMs[upgradableVMs[randomVMId]].VMDescr && 
                (!newVMType.has_value() || newVMType.value() < vmDescr)) {
                newVMType = vmDescr;
            }
        }
        xbt_assert(newVMType, "Something went wrong, can't upgrade VM type!");
        availableVMs[randomVMId].VMDescr = newVMType.value();
    }
    return !upgradableVMs.empty();
}

BaselineScheduler::Actions BaselineScheduler::MakeActions(const vector<VMData>& availableVMs) const {
    BaselineScheduler::Actions actions;
    Schedule s;

    for (int vmDataId = 0; vmDataId < static_cast<int>(availableVMs.size()); ++vmDataId) {
        VMData vmData = availableVMs[vmDataId];
        actions.push_back(std::make_shared<BuyVMAction>(vmData.VMDescr, vmDataId));
        for (int taskId: vmData.TasksQueue) {
            s.AddItem(vmDataId, ScheduleItem(taskId));
        }
    }

    actions.push_back(std::make_shared<ResetScheduleAction>(s));

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
