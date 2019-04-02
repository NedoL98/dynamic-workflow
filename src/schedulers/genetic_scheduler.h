#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

struct Assignment {
    std::vector<std::vector<int>> Schedule;
    std::optional<double> Cost;
    std::optional<double> Makespan;
    std::optional<double> FitnessScore;
};

class GeneticScheduler: public StaticScheduler {
friend class Assignment;

public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

private:
    std::vector<Assignment> GetInitialAssignments(View::Viewer& v, int numAssignments, std::vector<VMDescription>& availableVMs);

    double CalculateMakespan(const Assignment& assignment);
    double CalculateCost(const Assignment& assignment);

    double CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost);

    std::vector<VMDescription> AvailableVMs;
};
