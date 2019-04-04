#include "genetic_scheduler.h"

using std::max;
using std::pair;
using std::swap;
using std::vector;

GeneticScheduler::Actions GeneticScheduler::PrepareForRun(View::Viewer& v) {
    viewer = std::make_shared<View::Viewer>(v);

    for (VMDescription vmDecr: v.GetAvailiableVMTaxes()) {
        for (int i = 0; i < 50; ++i) {
            AvailableVMs.push_back(vmDecr);
        }
    }

    vector<Assignment> assignments = GetInitialAssignments(v, GENERATION_SIZE, AvailableVMs);

    FillAssignmentValues(assignments);

    for (int i = 0; i < 10; ++i) {
        double fitnessAvg = 0;
        for (Assignment& assignment: assignments) {
            fitnessAvg += assignment.FitnessScore.value();
        }
        fitnessAvg /= assignments.size();
        std::cout << "Epoch " << i << " fitness score average: " << fitnessAvg << std::endl; 

        assignments = GetNewGeneration(assignments);
        FillAssignmentValues(assignments);
    }
}

Assignment::Assignment(int n) {
    Schedule.resize(n);
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

double GeneticScheduler::CalculateMakespan(const Assignment& assignment) const {
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

                for (const int dependencyTaskId: viewer->GetTaskById(taskId).GetDependencies()) {
                    if (endTime[dependencyTaskId] == -1) {
                        canProcessTask = false;
                        break;
                    }
                    startTime = max(startTime, endTime[dependencyTaskId]);
                }

                if (canProcessTask) {
                    // TODO: enhance this when we can calculate data transition time
                    endTime[taskId] = startTime + (viewer->GetTaskById(taskId).GetTaskSpec().Cost / AvailableVMs[queueId].GetFlops());
                    makespan = max(endTime[taskId], makespan);
                    ++queueNextTask[queueId];
                    
                    ++completedTasks;
                    taskProcessed = true;
                }
            }
        }
        xbt_assert(taskProcessed, "Can't process any task, task order is inconsistent!");
    }

    return makespan;
}

double GeneticScheduler::CalculateCost(const Assignment& assignment) const {
    double cost = 0;
    for (int i = 0; i < assignment.Schedule.size(); ++i) {
        for (const int taskId: assignment.Schedule[i]) {
            View::Task task = viewer->GetTaskById(taskId);
            cost += AvailableVMs[i].GetPrice() * (task.GetTaskSpec().Cost / AvailableVMs[i].GetFlops());
        }
    }
    return cost;
}

double GeneticScheduler::CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost) const {
    if (!assignment.Cost || !assignment.Makespan) {
        xbt_assert("Fitness function can't be calculated without cost and makespan calculated!");
    }
    if (assignment.Makespan > viewer->GetDeadline()) {
        return (assignment.Makespan.value() / viewer->GetDeadline()) + 1;
    }
    return assignment.Cost.value() / maxGenerationCost;
}

void GeneticScheduler::FillAssignmentValues(vector<Assignment>& assignments) const {
    double maxCost = 0;
    for (Assignment& assignment: assignments) {
        assignment.Cost = CalculateCost(assignment);
        maxCost = max(maxCost, assignment.Cost.value());
        assignment.Makespan = CalculateMakespan(assignment);
    }

    for (Assignment& assignment: assignments) {
        assignment.FitnessScore = CalculateFitnessFunction(assignment, maxCost);
    }
}

pair<int, int> GeneticScheduler::GetRandomParents(const vector<Assignment>& parents) const {
    vector<bool> isPicked(parents.size());
    pair<int, int> parentsIds;
    pair<double, double> weights;
    double totalInvFitness = 0;

    for (int i = 0; i < 2; ++i) {
        double totalInvFitnessLocal = 0;
        for (int j = 0; j < parents.size(); ++j) {
            if (!isPicked[j]) {
                xbt_assert(parents[j].FitnessScore, "Fitness score is not calculated for some parents!");
                totalInvFitnessLocal += 1 / parents[j].FitnessScore.value();
            }
        }
        if (i == 0) {
            totalInvFitness = totalInvFitnessLocal;
        }

        double threshold = (rand() / static_cast<double>(RAND_MAX)) * totalInvFitnessLocal;
        double prefixFitness = 0;
        // binary search here may worth it
        for (int j = 0; j < parents.size(); ++j) {
            if (!isPicked[j]) {
                if (prefixFitness + (1 / parents[j].FitnessScore.value()) >= threshold) {
                    isPicked[j] = true;
                    if (i == 0) {
                        parentsIds.first = j;
                        weights.first = (1 / parents[j].FitnessScore.value());
                    } else {
                        parentsIds.second = j;
                        weights.second = (1 / parents[j].FitnessScore.value());
                    }
                    break;
                }
                prefixFitness += (1 / parents[j].FitnessScore.value());
            }
        }
    }

    // making 1->2 and 2->1 situation happen equally often
    double probPrime = (weights.first / totalInvFitness) * (weights.second / (totalInvFitness - weights.second));
    double probInverse = (weights.second / totalInvFitness) * (weights.first / (totalInvFitness - weights.first));
    if ((rand() / static_cast<double>(RAND_MAX)) <= probInverse / (probPrime + probInverse)) {
        swap(parentsIds.first, parentsIds.second);
    }

    return parentsIds;
}

Assignment GeneticScheduler::Crossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    int segmentBegin = rand() % viewer->GetTaskList().size();
    int segmentEnd = rand() % viewer->GetTaskList().size();
    if (segmentBegin > segmentEnd) {
        swap(segmentBegin, segmentEnd);
    }

    vector<int> taskIdToVM(viewer->GetTaskList().size());
    for (int vmId = 0; vmId < secondaryParent.Schedule.size(); ++vmId) {
        for (int taskId: secondaryParent.Schedule[vmId]) {
            taskIdToVM[taskId] = vmId;
        }
    }

    Assignment offspring(max(mainParent.Schedule.size(), secondaryParent.Schedule.size()));

    int itemInd = 0;
    for (int vmId = 0; vmId < mainParent.Schedule.size(); ++vmId) {
        for (int taskId: mainParent.Schedule[vmId]) {
            if (itemInd >= segmentBegin && itemInd <= segmentEnd) {
                offspring.Schedule[taskIdToVM[taskId]].push_back(taskId);
            } else {
                offspring.Schedule[vmId].push_back(taskId);
            }
            ++itemInd;
        }
    }

    return offspring;
}

vector<Assignment> GeneticScheduler::GetNewGeneration(const vector<Assignment>& oldGeneration) const {
    vector<Assignment> newGeneration;
    while (newGeneration.size() < GENERATION_SIZE) {
        pair<int, int> parentsIds = GetRandomParents(oldGeneration);
        newGeneration.push_back(Crossover(oldGeneration[parentsIds.first], oldGeneration[parentsIds.second]));
    }
    return newGeneration;
}
