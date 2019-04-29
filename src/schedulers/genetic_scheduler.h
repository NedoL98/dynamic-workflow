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

struct Mutation {
    std::function<void(Assignment&)> MutationFunc;
    double MutationProb;
};

struct Crossover {
    std::function<Assignment(const Assignment&, const Assignment&)> CrossoverFunc;
    double CrossoverProb;
};

class GeneticScheduler: public StaticScheduler {
public:
    GeneticScheduler();
    GeneticScheduler(const GeneticScheduler& other) = delete;
    GeneticScheduler& operator = (const GeneticScheduler& other) = delete;
    
    virtual Actions PrepareForRun(View::Viewer& v) override;

    static AbstractScheduler* Create() { return new GeneticScheduler(); };

private:
    void RegisterMutation(Mutation mutation);
    void RegisterCrossover(Crossover crossover);

    std::vector<Assignment> GetInitialAssignments(int numAssignments) const;

    std::vector<double> GetEndTimes(const Assignment& assignment) const;
    double CalculateMakespan(const Assignment& assignment) const;
    double CalculateCost(const Assignment& assignment) const;
    double CalculateFitnessFunction(const Assignment& assignment, double maxGenerationCost) const;

    void FillAssignmentValues(std::vector<Assignment>& assignments) const;

    std::pair<int, int> GetRandomParents(const std::vector<Assignment>& parents) const;
    Assignment GetMatchingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    Assignment GetSchedulingCrossover(const Assignment& mainParent, const Assignment& secondaryParent) const;
    void MakeMatchingMutation(Assignment& assignment) const;
    void MakeSchedulingMutation(Assignment& assignment) const;
    void MakeCrossover(std::function<Assignment(const Assignment&, const Assignment&)> crossover,
                       Assignment& assignment1, 
                       Assignment& assignment2) const;
    void MakeMutation(std::function<void(Assignment&)> mutation,
                      Assignment& assignment) const;

    std::vector<Assignment> GetBestChromosomes(const std::vector<Assignment>& generation) const;

    void DoModifyGeneration(std::vector<Assignment>& generation) const;
    std::vector<Assignment> GetNewGeneration(const std::vector<Assignment>& oldGeneration) const;

    double GetCheapestAssignment(std::vector<Assignment>& assignments) const;

    void PrintEpochStatistics(std::vector<Assignment>& assignments, int epochInd) const;

    void DoRefineAssignment(Assignment& assignment) const;
    void DoRefineAssignments(std::vector<Assignment>& assignments) const;

    std::vector<VMDescription> AvailableVMs;
    std::vector<Mutation> Mutations;
    std::vector<Crossover> Crossovers;
    static constexpr int InitFastestAssignmentsNum = 5;
    static constexpr int GenerationSize = 50;
    static constexpr int StepsNumber = 10 * 1000;
    static constexpr int ConstantScoreStepsNumber = 300;

    static constexpr int BestChromosomesNumber = 5;
    static constexpr double MatchingCrossoverProb = 0.5;
    static constexpr double SchedulingCrossoverProb = 0.5;
    static constexpr double MatchingMutationProb = 0.2;
    static constexpr double SchedulingMutationProb = 0.2;
};
