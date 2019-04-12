#include "genetic_scheduler.h"

using std::max;
using std::min;
using std::pair;
using std::set;
using std::sort;
using std::swap;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(genetic_scheduler, "Genetic scheduler log");

GeneticScheduler::Actions GeneticScheduler::PrepareForRun(View::Viewer& v) {
    viewer = std::make_shared<View::Viewer>(v);

    for (VMDescription vmDecr: v.GetAvailiableVMTaxes()) {
        for (int i = 0; i < 50; ++i) {
            AvailableVMs.push_back(vmDecr);
        }
    }

    vector<Assignment> assignments = GetInitialAssignments(GENERATION_SIZE);

    FillAssignmentValues(assignments);

    for (int i = 0; i < 10; ++i) {
        assignments = GetNewGeneration(assignments);
        FillAssignmentValues(assignments);
        PrintEpochStatistics(assignments, i);
    }
}

Assignment::Assignment(int n) {
    MatchingString.resize(n);
    SchedulingString.resize(n);
}

Assignment::Assignment(const Assignment& other) {
    MatchingString = other.MatchingString;
    SchedulingString = other.SchedulingString;
}

vector<Assignment> GeneticScheduler::GetInitialAssignments(int numAssignments) {
    vector<View::Task> tasksOrder = viewer->MakeTasksOrder();
    vector<Assignment> initialAssignments;
    initialAssignments.resize(numAssignments);
    for (int i = 0; i < numAssignments; ++i) {
        initialAssignments[i] = Assignment(viewer->GetTaskList().size());
        
        transform(tasksOrder.begin(), tasksOrder.end(), initialAssignments[i].SchedulingString.begin(), 
                [](const View::Task &task) { return task.GetId(); });

        int ssMutationNumber = rand() % viewer->GetTaskList().size();
        while (ssMutationNumber) {
            MakeSchedulingMutation(initialAssignments[i]);
            --ssMutationNumber;
        }

        for (int taskId = 0; taskId < viewer->GetTaskList().size(); ++taskId) {
            vector<int> suitableVMs;
            for (int i = 0; i < AvailableVMs.size(); ++i) {
                if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[i])) {
                    suitableVMs.push_back(i);
                }
            }
            xbt_assert(!suitableVMs.empty(), "No suitable VM found for task %s", viewer->GetTaskById(taskId).GetName().c_str());
            int randomVMId = rand() % suitableVMs.size();
            initialAssignments[i].MatchingString[taskId] = suitableVMs[randomVMId];
        }
    }
    return initialAssignments;
}

vector<vector<int>> GeneticScheduler::Get2DSchedule(const Assignment& assignment) const {
    vector<vector<int>> schedule(AvailableVMs.size());

    for (int taskId: assignment.SchedulingString) {
        schedule[assignment.MatchingString[taskId]].push_back(taskId);
    }

    return schedule;
}

double GeneticScheduler::CalculateMakespan(const Assignment& assignment) const {
    double makespan = 0;

    vector<double> endTime(viewer->GetTaskList().size(), -1);

    vector<vector<int>> schedule = Get2DSchedule(assignment);

    int completedTasks = 0;
    vector<int> queueNextTask(AvailableVMs.size());

    while (completedTasks != viewer->GetTaskList().size()) {
        bool taskProcessed = false;
        for (int queueId = 0; queueId < schedule.size(); ++queueId) {
            if (queueNextTask[queueId] < schedule[queueId].size()) {
                int taskId = schedule[queueId][queueNextTask[queueId]];
                
                bool canProcessTask = true;
                double startTime = 0;
                if (queueNextTask[queueId] != 0) {
                    startTime = endTime[schedule[queueId][queueNextTask[queueId] - 1]];
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
    for (int taskId = 0; taskId < viewer->GetTaskList().size(); ++taskId) {
        View::Task task = viewer->GetTaskById(taskId);
        int vmId = assignment.MatchingString[taskId];
        cost += AvailableVMs[vmId].GetPrice() * (task.GetTaskSpec().Cost / AvailableVMs[vmId].GetFlops());
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

Assignment GeneticScheduler::MatchingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    Assignment offspring(mainParent);

    int cutoff = rand() % viewer->GetTaskList().size();
    for (int i = cutoff; i < viewer->GetTaskList().size(); ++i) {
        int taskId = offspring.SchedulingString[i];
        int newVMId = secondaryParent.MatchingString[taskId];
        if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[newVMId])) {
            offspring.MatchingString[taskId] = newVMId;
        }
    }

    return offspring;
}

Assignment GeneticScheduler::SchedulingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    Assignment offspring(mainParent);

    vector<int> taskIdToPosition(viewer->GetTaskList().size());
    for (int i = 0; i < viewer->GetTaskList().size(); ++i) {
        taskIdToPosition[secondaryParent.SchedulingString[i]] = i;
    }

    int cutoff = rand() % viewer->GetTaskList().size();

    sort(offspring.SchedulingString.begin() + cutoff, offspring.SchedulingString.end(), [&taskIdToPosition](int taskId1, int taskId2) {
        return taskIdToPosition[taskId1] < taskIdToPosition[taskId2];
    });

    return offspring;
}

void GeneticScheduler::MakeMatchingMutation(Assignment& assignment) const {
    int taskId = rand() % viewer->GetTaskList().size();

    vector<int> suitableVMs;
    for (int i = 0; i < AvailableVMs.size(); ++i) {
        if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[i])) {
            suitableVMs.push_back(i);
        }
    }
    xbt_assert(!suitableVMs.empty(), "No suitable VM found for task %s", viewer->GetTaskById(taskId).GetName().c_str());
    int randomVMId = rand() % suitableVMs.size();
    assignment.MatchingString[taskId] = suitableVMs[randomVMId];
}

void GeneticScheduler::MakeSchedulingMutation(Assignment& assignment) const {
    int mutationTaskPosition = rand() % viewer->GetTaskList().size();
    int mutationTaskId = assignment.SchedulingString[mutationTaskPosition];

    int lowerBound = viewer->GetTaskList().size() - 1;
    int upperBound = viewer->GetTaskList().size();

    set<int> dependencies(viewer->GetTaskById(mutationTaskId).GetDependencies().begin(), viewer->GetTaskById(mutationTaskId).GetDependencies().end());
    set<int> successors(viewer->GetTaskById(mutationTaskId).GetSuccessors().begin(), viewer->GetTaskById(mutationTaskId).GetSuccessors().end());

    for (int i = 0; i < viewer->GetTaskList().size(); ++i) {
        if (dependencies.empty()) {
            lowerBound = min(lowerBound, i);
        }
        dependencies.erase(assignment.SchedulingString[i]);
        if (successors.count(assignment.SchedulingString[i])) {
            upperBound = i;
            break;
        }
    }
    xbt_assert(lowerBound < upperBound, "Task order is inconsitent!");

    int newTaskPosition = lowerBound + (rand() % (upperBound - lowerBound));
    if (newTaskPosition > mutationTaskPosition) {
        --newTaskPosition;
    }

    assignment.SchedulingString.erase(assignment.SchedulingString.begin() + mutationTaskPosition);
    assignment.SchedulingString.insert(assignment.SchedulingString.begin() + newTaskPosition, mutationTaskId);
}

vector<Assignment> GeneticScheduler::GetNewGeneration(const vector<Assignment>& oldGeneration) const {
    vector<Assignment> newGeneration;
    while (newGeneration.size() < GENERATION_SIZE) {
        pair<int, int> parentsIds = GetRandomParents(oldGeneration);
        if ((rand() / static_cast<double>(RAND_MAX)) * (MatchingCrossoverProb + SchedulingCrossoverProb) <= MatchingCrossoverProb) {
            newGeneration.push_back(MatchingCrossover(oldGeneration[parentsIds.first], oldGeneration[parentsIds.second]));
        } else {
            newGeneration.push_back(SchedulingCrossover(oldGeneration[parentsIds.first], oldGeneration[parentsIds.second]));
        }
    }

    for (Assignment& assignment: newGeneration) {
        double probTmp = rand() / static_cast<double>(RAND_MAX);
        if (probTmp <= MatchingMutationProb) {
            MakeMatchingMutation(assignment);
        } else if (probTmp <= MatchingMutationProb + SchedulingMutationProb) {
            MakeSchedulingMutation(assignment);
        }
    }

    return newGeneration;
}

void GeneticScheduler::PrintEpochStatistics(vector<Assignment>& assignments, int epochInd) const {
    double fitnessAvg = 0;
    double cheapestAssignment = -1;
    for (Assignment& assignment: assignments) {
        fitnessAvg += assignment.FitnessScore.value();
        if (cheapestAssignment == -1 || cheapestAssignment > assignment.Cost.value()) {
            cheapestAssignment = assignment.Cost.value();
        }
    }
    fitnessAvg /= assignments.size();
    XBT_INFO("Epoch %d: ", epochInd);
    XBT_INFO("Fitness score average: %f", fitnessAvg);
    XBT_INFO("Cheapest assignment: %f", cheapestAssignment);
}
