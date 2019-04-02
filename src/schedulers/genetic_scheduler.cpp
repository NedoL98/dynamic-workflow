#include "genetic_scheduler.h"

using std::max;
using std::vector;

GeneticScheduler::Actions GeneticScheduler::PrepareForRun(View::Viewer& v) {
    viewer = std::make_shared<View::Viewer>(v);

    for (VMDescription vmDecr: v.GetAvailiableVMTaxes()) {
        for (int i = 0; i < 5; ++i) {
            AvailableVMs.push_back(vmDecr);
        }
    }

    vector<Assignment> assignments = GetInitialAssignments(v, 5, AvailableVMs);

    double maxCost = 0;
    for (Assignment& assignment: assignments) {
        assignment.Cost = CalculateCost(assignment);
        assignment.Makespan = CalculateCost(assignment);
        maxCost = max(maxCost, assignment.Cost.value());
    }

    for (Assignment& assignment: assignments) {
        assignment.FitnessScore = CalculateFitnessFunction(assignment, maxCost);
    }
}

vector<Assignment> GeneticScheduler::GetInitialAssignments(View::Viewer& v, 
                                                           int numAssignments,
                                                           vector<VMDescription>& availableVMs) {
    vector<View::Task> tasksOrder = v.MakeTasksOrder();
    vector<Assignment> initialAssignments;
    initialAssignments.resize(numAssignments);
    for (int i = 0; i < numAssignments; ++i) {
        vector<vector<int>> currentAssignment(availableVMs.size());

        vector<bool> isAssigned(v.GetTaskList().size()); // for debug only
        for (const View::Task& task: tasksOrder) {
            for (const int dependencyId: task.GetDependencies()) {
                xbt_assert(isAssigned[dependencyId], "Tasks order is inconsistent!");
            }
            vector<int> suitableVMs;
            for (int i = 0; i < availableVMs.size(); ++i) {
                if (task.CanBeExecuted(availableVMs[i])) {
                    suitableVMs.push_back(i);
                }
            }
            xbt_assert(!suitableVMs.empty(), "No suitable VM found for task %s", task.GetName().c_str());
            int randomVMId = rand() % suitableVMs.size();
            currentAssignment[randomVMId].push_back(task.GetId());
            isAssigned[task.GetId()] = true;
        }

        initialAssignments[i].Schedule = currentAssignment;
    }
    return initialAssignments;
}

double GeneticScheduler::CalculateMakespan(const Assignment& assignment) {
    double makespan = 0;

    vector<double> endTime(viewer->GetTaskList().size(), -1);

    int completedTasks = 0;
    vector<int> queueNextTask(assignment.Schedule.size());

    while (completedTasks != viewer->GetTaskList().size()) {
        bool taskProcessed = false;
        for (int queueId = 0; queueId < assignment.Schedule.size(); ++queueId) {
            if (queueNextTask[queueId] < assignment.Schedule[queueId].size()) {
                int taskId = assignment.Schedule[queueId][queueNextTask[queueId]];
                
                bool canProcessTask = true;
                double startTime = 0;
                if (queueNextTask[queueId] != 0) {
                    startTime = endTime[assignment.Schedule[queueId][queueNextTask[queueId] - 1]];
                }

                for (const int taskId: viewer->GetTaskById(taskId).GetDependencies()) {
                    if (endTime[taskId] == -1) {
                        canProcessTask = false;
                        break;
                    }
                    startTime = max(startTime, endTime[taskId]);
                }

                if (canProcessTask) {
                    // TODO: enhance this when we can calculate data transition time
                    endTime[taskId] = startTime + (viewer->GetTaskById(taskId).GetTaskSpec().Cost / AvailableVMs[queueId].GetFlops());

                    taskProcessed = true;
                }
            }
        }
        xbt_assert(taskProcessed, "Can't process any task, task order is inconsistent!");
    }

    return makespan;
}

double GeneticScheduler::CalculateCost(const Assignment& assignment) {
    double cost = 0;
    for (int i = 0; i < assignment.Schedule.size(); ++i) {
        for (const int taskId: assignment.Schedule[i]) {
            View::Task task = viewer->GetTaskById(taskId);
            cost += AvailableVMs[i].GetPrice() * (task.GetTaskSpec().Cost / AvailableVMs[i].GetFlops());
        }
    }
    return cost;
}

double GeneticScheduler::CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost) {
    if (!assignment.Cost || !assignment.Makespan) {
        xbt_assert("Fitness function can't be calculated without cost and makespan calculated!");
    }
    if (assignment.Makespan > viewer->GetDeadline()) {
        return (assignment.Makespan.value() / viewer->GetDeadline()) + 1;
    }
    return assignment.Cost.value() / maxGenerationCost;
}