#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

class GeneticScheduler;

struct Assignment {
    Assignment() = default;
    Assignment(int n);

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
    Assignment GetMatchingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    Assignment GetSchedulingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    void MakeCrossover(Assignment& assignment1, 
                       Assignment& assignment2, 
                       std::function<Assignment(const Assignment&, const Assignment&)> crossover) const;

    void MakeMatchingMutation(Assignment& assignment) const;
    void MakeSchedulingMutation(Assignment& assignment) const;

    std::vector<Assignment> GetBestChromosomes(const std::vector<Assignment>& generation) const;

    std::vector<Assignment> GetNewGeneration(const std::vector<Assignment>& oldGeneration) const;

    void PrintEpochStatistics(std::vector<Assignment>& assignments, int epochInd) const;

    std::vector<VMDescription> AvailableVMs;
    const int GENERATION_SIZE = 50;
    const int NUM_VMS = 50;
    const int NUM_STEPS = 10 * 1000;

    const int BestChromosomesNumber = 5;
    const double MatchingCrossoverProb = 0.5;
    const double SchedulingCrossoverProb = 0.5;
    const double MatchingMutationProb = 0.2;
    const double SchedulingMutationProb = 0.2;
};
