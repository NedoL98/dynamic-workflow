#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

class GeneticScheduler;

struct Assignment {
    Assignment() = default;
    Assignment(int n);

    std::vector<std::vector<int>> Schedule;
    std::optional<double> Cost;
    std::optional<double> Makespan;
    std::optional<double> FitnessScore;
};

class GeneticScheduler: public StaticScheduler {
public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

private:
    std::vector<Assignment> GetInitialAssignments(View::Viewer& v, int numAssignments, std::vector<VMDescription>& availableVMs);

    double CalculateMakespan(const Assignment& assignment) const;
    double CalculateCost(const Assignment& assignment) const;
    double CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost) const;

    void FillAssignmentValues(std::vector<Assignment>& assignments) const;

    std::vector<VMDescription> AvailableVMs;

    std::pair<int, int> GetRandomParents(const std::vector<Assignment>& parents) const;
    Assignment Crossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    std::vector<Assignment> GetNewGeneration(const std::vector<Assignment>& oldGeneration) const;

    const int GENERATION_SIZE = 10; 
};
