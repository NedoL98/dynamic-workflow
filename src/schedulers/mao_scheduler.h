#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

struct LoadVectorEvent {
    double ConsumptionRatio;
    double Begin;
    double End;
    int TaskId;
    int VMId;
};

class MaoScheduler: public StaticScheduler {
public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

    void MakeOrderDFS(int v, std::vector<View::Task>& order, std::vector<bool>& used);
    std::vector<View::Task> MakeTasksOrder();

    void ReduceMakespan(std::vector<VMDescription>& taskVM, const std::vector<View::Task>& taskOrder, double currentMakespan);


    std::vector<double> CalculateTasksEndTimes(const std::vector<View::Task>& taskOrder,
                                               const std::vector<VMDescription>& taskVM);
    std::vector<std::pair<double, double>> CalculateDeadlines(View::Viewer& v,
                                                              const std::vector<View::Task>& taskOrder,
                                                              const std::vector<VMDescription>& taskVM);

    std::vector<std::vector<LoadVectorEvent>> GetLoadVector(const std::vector<std::pair<double, double>>& deadlines,
                                                            const std::vector<VMDescription>& taskVM);

    std::vector<VMDescription> GetCheapestVMs();
};
