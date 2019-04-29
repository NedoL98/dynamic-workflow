#include "genetic_scheduler.h"

using namespace std::placeholders;

using std::bind;
using std::function;
using std::max;
using std::max_element;
using std::min;
using std::min_element;
using std::pair;
using std::set;
using std::sort;
using std::swap;
using std::upper_bound;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(genetic_scheduler, "Genetic scheduler log");

GeneticScheduler::Actions GeneticScheduler::PrepareForRun(View::Viewer& v) {
    viewer = std::make_shared<View::Viewer>(v);

    for (VMDescription vmDecr: v.GetVMList()) {
        for (size_t i = 0; i < v.WorkflowSize(); ++i) {
            AvailableVMs.push_back(vmDecr);
        }
    }

    vector<Assignment> assignments = GetInitialAssignments(GenerationSize);

    FillAssignmentValues(assignments);

    int constantScoreSteps = 0;
    double previousScore = -1;
    for (int i = 0; i < StepsNumber; ++i) {
        assignments = GetNewGeneration(assignments);
        FillAssignmentValues(assignments);
        if (i % 100 == 0) {
            PrintEpochStatistics(assignments, i);
        }
        double currentScore = GetCheapestAssignment(assignments);
        if (previousScore != currentScore) {
            previousScore = currentScore;
            constantScoreSteps = 0;
        } else if (previousScore != -1) {
            ++constantScoreSteps;
        }

        if (constantScoreSteps == ConstantScoreStepsNumber) {
            break;
        }
    }

    Assignment bestAssignment;
    for (const Assignment& assignment: assignments) {
        if (!bestAssignment.FitnessScore.has_value() || assignment.FitnessScore.value() < bestAssignment.FitnessScore.value()) {
            bestAssignment = assignment;
        }
    }

    vector<bool> nonEmptyVMs(AvailableVMs.size(), false);
    for (int vmId: bestAssignment.MatchingString) {
        nonEmptyVMs[vmId] = true;
    }

    GeneticScheduler::Actions actions;

    Schedule s;

    vector<int> idOldToNew(AvailableVMs.size(), -1);
    int vmNewId = 0;
    for (size_t vmOldId = 0; vmOldId < AvailableVMs.size(); ++vmOldId) {
        if (nonEmptyVMs[vmOldId]) {
            VMDescription vmDescr = AvailableVMs[vmOldId];
            idOldToNew[vmOldId] = vmNewId;
            actions.push_back(std::make_shared<BuyVMAction>(vmDescr, vmNewId));
            ++vmNewId;
        }
    }
    for (int taskId: bestAssignment.SchedulingString) {
        s.AddItem(idOldToNew[bestAssignment.MatchingString[taskId]], ScheduleItem(taskId));
    }

    actions.push_back(std::make_shared<ResetScheduleAction>(s));

    XBT_INFO("Schedule created!");
    XBT_INFO("Makespan: %f", bestAssignment.Makespan.value());
    XBT_INFO("Cost: %f", bestAssignment.Cost.value());

    return actions;
}

Assignment::Assignment(int n) {
    MatchingString.resize(n);
    SchedulingString.resize(n);
    KeepVMStandbyString.resize(n);
}

GeneticScheduler::GeneticScheduler() {
    RegisterCrossover({bind(&GeneticScheduler::GetSchedulingCrossover, this, _1, _2), SchedulingCrossoverProb});
    RegisterCrossover({bind(&GeneticScheduler::GetMatchingCrossover, this, _1, _2), MatchingCrossoverProb});
    RegisterCrossover({bind(&GeneticScheduler::GetKeepVMStandbyCrossover, this, _1, _2), KeepVMStandbyCrossoverProb});
    RegisterMutation({bind(&GeneticScheduler::MakeSchedulingMutation, this, _1), SchedulingMutationProb});
    RegisterMutation({bind(&GeneticScheduler::MakeMatchingMutation, this, _1), MatchingMutationProb});
    RegisterMutation({bind(&GeneticScheduler::MakeKeepVMStandbyMutation, this, _1), KeepVMStandbyMutationProb});
}

void GeneticScheduler::RegisterMutation(Mutation mutation) {
    Mutations.push_back(std::move(mutation));
}

void GeneticScheduler::RegisterCrossover(Crossover crossover) {
    Crossovers.push_back(std::move(crossover));
}

vector<Assignment> GeneticScheduler::GetInitialAssignments(int numAssignments) const {
    vector<View::Task> tasksOrder = viewer->MakeTasksOrder();
    vector<Assignment> initialAssignments;
    initialAssignments.resize(numAssignments);
    for (int i = 0; i < numAssignments; ++i) {
        initialAssignments[i] = Assignment(viewer->WorkflowSize());

        transform(tasksOrder.begin(), tasksOrder.end(), initialAssignments[i].SchedulingString.begin(), 
                [](const View::Task &task) { return task.GetId(); });

        int ssMutationNumber = rand() % viewer->WorkflowSize();
        while (ssMutationNumber) {
            MakeSchedulingMutation(initialAssignments[i]);
            --ssMutationNumber;
        }
        
        for (size_t taskId = 0; taskId < viewer->WorkflowSize(); ++taskId) {
            vector<int> suitableVMs;
            for (size_t vmId = 0; vmId < AvailableVMs.size(); ++vmId) {
                if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[vmId])) {
                    // explicitly adding a fastest possible task allocation
                    if (i >= InitFastestAssignmentsNum) {
                        suitableVMs.push_back(vmId);                        
                    } else if (suitableVMs.empty() || AvailableVMs[suitableVMs.front()] <= AvailableVMs[vmId]) {
                        if (!suitableVMs.empty() && AvailableVMs[suitableVMs.front()] < AvailableVMs[vmId]) {
                            suitableVMs.clear();
                        }
                        suitableVMs.push_back(vmId);
                    }
                }
            }
            xbt_assert(!suitableVMs.empty(), "No suitable VM found for task %s", viewer->GetTaskById(taskId).GetName().c_str());
            int randomVMId = rand() % suitableVMs.size();
            initialAssignments[i].MatchingString[taskId] = suitableVMs[randomVMId];

            if (i >= InitFastestAssignmentsNum) {
                initialAssignments[i].KeepVMStandbyString[taskId] = rand() % 2;
            } else {
                initialAssignments[i].KeepVMStandbyString[taskId] = true;
            }
        }
    }
    return initialAssignments;
}

void GeneticScheduler::DoCalculateCostAndMakespan(Assignment& assignment) const {
    vector<double> endTimes(viewer->WorkflowSize(), -1);
    vector<int> prevTaskId(AvailableVMs.size(), -1);

    double avgLag = static_cast<double>(viewer->GetVMList().GetMinLag() + viewer->GetVMList().GetMaxLag()) / 2;
    
    assignment.Makespan = 0;
    assignment.Cost = 0;

    for (size_t i = 0; i < assignment.SchedulingString.size(); ++i) {
        int taskId = assignment.SchedulingString[i];
        int vmId = assignment.MatchingString[taskId];
        int prevVMTaskId = prevTaskId[vmId];

        double earliestBegin = 0;
        if (prevVMTaskId != -1) {
            earliestBegin = endTimes[prevVMTaskId];
        }
        for (const int dependencyTaskId: viewer->GetTaskById(taskId).GetDependencies()) {
            xbt_assert(endTimes[dependencyTaskId] != -1, "Can't process next task, task order is inconsistent!");
            if (dependencyTaskId != prevVMTaskId) {
                earliestBegin = max(earliestBegin, endTimes[dependencyTaskId]);
            }
        }
        if (prevVMTaskId == -1 || !assignment.KeepVMStandbyString[prevVMTaskId]) {
            earliestBegin += avgLag;
        }
        VMDescription vmDescr = AvailableVMs[assignment.MatchingString[taskId]];
        endTimes[taskId] = earliestBegin + viewer->GetTaskById(taskId).GetExecutionTime(vmDescr);

        assignment.Makespan = max(assignment.Makespan.value(), endTimes[taskId]);
        assignment.Cost = assignment.Cost.value() + viewer->GetTaskById(taskId).GetExecutionCost(AvailableVMs[vmId]);
        if (prevVMTaskId != -1 && assignment.KeepVMStandbyString[prevVMTaskId]) {
            assignment.Cost = assignment.Cost.value() + AvailableVMs[vmId].GetPrice() * (earliestBegin - endTimes[prevVMTaskId]);
        }

        prevTaskId[vmId] = taskId;
    }
}

void GeneticScheduler::DoCalculateFitnessFunction(Assignment& assignment, double maxGenerationCost) const {
    if (!assignment.Cost || !assignment.Makespan) {
        xbt_assert("Fitness function can't be calculated without cost and makespan calculated!");
    }
    if (assignment.Makespan.value() > viewer->GetDeadline()) {
        assignment.FitnessScore = (assignment.Makespan.value() / viewer->GetDeadline()) + 1;
    } else {
        assignment.FitnessScore = assignment.Cost.value() / maxGenerationCost;
    }
}

void GeneticScheduler::FillAssignmentValues(vector<Assignment>& assignments) const {
    double maxCost = 0;
    for (Assignment& assignment: assignments) {
        DoCalculateCostAndMakespan(assignment);
        maxCost = max(maxCost, assignment.Cost.value());
    }

    for (Assignment& assignment: assignments) {
        DoCalculateFitnessFunction(assignment, maxCost);
    }
}

pair<int, int> GeneticScheduler::GetRandomParents(const vector<Assignment>& parents) const {
    vector<bool> isPicked(parents.size());
    pair<int, int> parentsIds;
    pair<double, double> weights;
    double totalInvFitness = 0;

    for (int i = 0; i < 2; ++i) {
        double totalInvFitnessLocal = 0;
        for (size_t j = 0; j < parents.size(); ++j) {
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
        // FIXME: binary search here may worth it
        for (size_t j = 0; j < parents.size(); ++j) {
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

Assignment GeneticScheduler::GetMatchingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    Assignment offspring(mainParent);

    int cutoff = rand() % viewer->WorkflowSize();
    for (size_t i = cutoff; i < viewer->WorkflowSize(); ++i) {
        int taskId = offspring.SchedulingString[i];
        int newVMId = secondaryParent.MatchingString[taskId];
        if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[newVMId])) {
            offspring.MatchingString[taskId] = newVMId;
        }
    }

    return offspring;
}

Assignment GeneticScheduler::GetSchedulingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    Assignment offspring(mainParent);

    vector<int> taskIdToPosition(viewer->WorkflowSize());
    for (size_t i = 0; i < viewer->WorkflowSize(); ++i) {
        taskIdToPosition[secondaryParent.SchedulingString[i]] = i;
    }

    int cutoff = rand() % viewer->WorkflowSize();

    sort(offspring.SchedulingString.begin() + cutoff, offspring.SchedulingString.end(), [&taskIdToPosition](int taskId1, int taskId2) {
        return taskIdToPosition[taskId1] < taskIdToPosition[taskId2];
    });

    return offspring;
}

Assignment GeneticScheduler::GetKeepVMStandbyCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const {
    Assignment offspring(mainParent);

    for (size_t taskId = 0; taskId < viewer->WorkflowSize(); ++taskId) {
        if (rand() % 10 == 0) {
            offspring.KeepVMStandbyString[taskId] = secondaryParent.KeepVMStandbyString[taskId];
        }
    }
    
    return offspring;
}

void GeneticScheduler::MakeMatchingMutation(Assignment& assignment) const {
    int taskId = rand() % viewer->WorkflowSize();

    vector<int> suitableVMs;
    for (size_t i = 0; i < AvailableVMs.size(); ++i) {
        if (viewer->GetTaskById(taskId).CanBeExecuted(AvailableVMs[i])) {
            suitableVMs.push_back(i);
        }
    }
    xbt_assert(!suitableVMs.empty(), "No suitable VM found for task %s", viewer->GetTaskById(taskId).GetName().c_str());
    int randomVMId = rand() % suitableVMs.size();
    assignment.MatchingString[taskId] = suitableVMs[randomVMId];
}

void GeneticScheduler::MakeSchedulingMutation(Assignment& assignment) const {
    int mutationTaskPosition = rand() % viewer->WorkflowSize();
    int mutationTaskId = assignment.SchedulingString[mutationTaskPosition];

    int lowerBound = viewer->WorkflowSize() - 1;
    int upperBound = viewer->WorkflowSize();

    set<int> dependencies(viewer->GetTaskById(mutationTaskId).GetDependencies().begin(), viewer->GetTaskById(mutationTaskId).GetDependencies().end());
    set<int> successors(viewer->GetTaskById(mutationTaskId).GetSuccessors().begin(), viewer->GetTaskById(mutationTaskId).GetSuccessors().end());

    for (int i = 0; i < static_cast<int>(viewer->WorkflowSize()); ++i) {
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

void GeneticScheduler::MakeKeepVMStandbyMutation(Assignment& assignment) const {
    int taskId = rand() % viewer->WorkflowSize();
    assignment.KeepVMStandbyString[taskId] = 1 - assignment.KeepVMStandbyString[taskId];
}

void GeneticScheduler::MakeCrossover(function<Assignment(const Assignment&, const Assignment&)> crossover,
                                     Assignment& assignment1, 
                                     Assignment& assignment2) const {
    Assignment tmpAssignment = crossover(assignment1, assignment2);
    assignment2 = crossover(assignment2, assignment1);
    assignment1 = tmpAssignment;
}

void GeneticScheduler::MakeMutation(function<void(Assignment&)> mutation,
                                    Assignment& assignment) const {
    mutation(assignment);
}

vector<Assignment> GeneticScheduler::GetBestChromosomes(const vector<Assignment>& generation) const {
    vector<Assignment> result(BestChromosomesNumber);
    std::partial_sort_copy(generation.begin(), generation.end(), result.begin(), result.end(), [](const Assignment& a, const Assignment& b) {
        xbt_assert(a.FitnessScore.has_value() && b.FitnessScore.has_value(), "Not every assignment has a calculated fitness value!");
        return a.FitnessScore < b.FitnessScore;
    });
    return result;
}

void GeneticScheduler::DoModifyGeneration(vector<Assignment>& generation) const {
    for (const Crossover& crossover: Crossovers) {
        int pairsCount = generation.size() * (generation.size() - 1) / 2;
        for (int i = 0; i < pairsCount * crossover.CrossoverProb; ++i) {
            MakeCrossover(crossover.CrossoverFunc, generation[rand() % generation.size()], generation[rand() % generation.size()]);
        }
    }

    for (const Mutation& mutation: Mutations) {
        for (int i = 0; i < generation.size() * mutation.MutationProb; ++i) {
            MakeMutation(mutation.MutationFunc, generation[rand() % generation.size()]);
        }
    }
}

vector<Assignment> GeneticScheduler::GetNewGeneration(const vector<Assignment>& oldGeneration) const {
    double totalInvFitness = 0;
    vector<double> prefInvFitness;
    for (const Assignment& assignment: oldGeneration) {
        totalInvFitness += 1 / assignment.FitnessScore.value();
        prefInvFitness.push_back(totalInvFitness);
    }

    vector<Assignment> newGeneration;

    while (newGeneration.size() != oldGeneration.size() - BestChromosomesNumber) {
        double randCutoff = (rand() / static_cast<double>(RAND_MAX)) * totalInvFitness;
        double assignmentId = upper_bound(prefInvFitness.begin(), prefInvFitness.end(), randCutoff) - prefInvFitness.begin();
        newGeneration.push_back(oldGeneration[assignmentId]);
    }

    DoModifyGeneration(newGeneration);

    vector<Assignment> bestOldAssignments = GetBestChromosomes(oldGeneration); 
    newGeneration.insert(newGeneration.end(), bestOldAssignments.begin(), bestOldAssignments.end());
    
    return newGeneration;
}

double GeneticScheduler::GetCheapestAssignment(vector<Assignment>& assignments) const {
    auto minElemPtr = min_element(assignments.begin(), assignments.end(), [](const Assignment& a, const Assignment& b) {
        if (a.FitnessScore > 1) {
            return false;
        } else if (b.FitnessScore > 1) {
            return true;
        }
        return a.Cost < b.Cost;
    });

    if (minElemPtr->Makespan.value() >= viewer->GetDeadline()) {
        return -1;
    }
    return minElemPtr->Cost.value();
}

void GeneticScheduler::PrintEpochStatistics(vector<Assignment>& assignments, int epochInd) const {
    double fitnessAvg = 0;
    double cheapestAssignment = -1;
    double fitnessBest = -1;
    double fitnessWorst = -1;
    for (Assignment& assignment: assignments) {
        fitnessAvg += assignment.FitnessScore.value();
        if ((cheapestAssignment == -1 || cheapestAssignment > assignment.Cost.value()) && assignment.Makespan <= viewer->GetDeadline()) {
            cheapestAssignment = assignment.Cost.value();
        }
        if (fitnessBest == -1 || fitnessBest > assignment.FitnessScore.value()) {
            fitnessBest = assignment.FitnessScore.value();
        }
        if (fitnessWorst == -1 || fitnessWorst < assignment.FitnessScore.value()) {
            fitnessWorst = assignment.FitnessScore.value();
        }
    }
    fitnessAvg /= assignments.size();
    XBT_INFO("Epoch %d: ", epochInd);
    XBT_INFO("Fitness score average: %f", fitnessAvg);
    XBT_INFO("Fitness score best: %f", fitnessBest);
    XBT_INFO("Fitness score worst: %f", fitnessWorst);
    XBT_INFO("Cheapest assignment: %f", cheapestAssignment);
}

void GeneticScheduler::DoRefineAssignment(Assignment& assignment) const {
    (void)assignment;
}

void GeneticScheduler::DoRefineAssignments(vector<Assignment>& assignments) const {
    for (Assignment& assignment: assignments) {
        DoRefineAssignment(assignment);
    }
}
