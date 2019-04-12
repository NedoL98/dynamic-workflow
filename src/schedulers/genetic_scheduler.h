#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

class GeneticScheduler;

struct Assignment {
    Assignment() = default;
    Assignment(int n);
    Assignment(const Assignment& other);

    std::vector<int> MatchingString;
    std::vector<int> SchedulingString;
    std::optional<double> Cost;
    std::optional<double> Makespan;
    std::optional<double> FitnessScore;
};

class GeneticScheduler: public StaticScheduler {
public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

private:
    std::vector<Assignment> GetInitialAssignments(int numAssignments);

    std::vector<std::vector<int>> Get2DSchedule(const Assignment& assignment) const;

    double CalculateMakespan(const Assignment& assignment) const;
    double CalculateCost(const Assignment& assignment) const;
    double CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost) const;

    void FillAssignmentValues(std::vector<Assignment>& assignments) const;

    std::pair<int, int> GetRandomParents(const std::vector<Assignment>& parents) const;
    Assignment MatchingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    Assignment SchedulingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;

    void MakeMatchingMutation(Assignment& assignment) const;
    void MakeSchedulingMutation(Assignment& assignment) const;

    std::vector<Assignment> GetNewGeneration(const std::vector<Assignment>& oldGeneration) const;

    void PrintEpochStatistics(std::vector<Assignment>& assignments, int epochInd) const;

    std::vector<VMDescription> AvailableVMs;
    const int GENERATION_SIZE = 10;
    const double MatchingCrossoverProb = 0.5;
    const double SchedulingCrossoverProb = 0.5;
    const double MatchingMutationProb = 0.2;
    const double SchedulingMutationProb = 0.2;
};
