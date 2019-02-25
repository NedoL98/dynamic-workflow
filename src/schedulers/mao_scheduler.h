#pragma once
#include "scheduler_base.h"

struct LoadVectorEvent {
    double ConsumptionRatio;
    double Begin;
    double End;
};

class MaoScheduler: public BaseScheduler {
public:
    MaoScheduler(const std::string& workflowPath, const std::string& vmListPath);

    void ProcessTasksGraph() override;

    double CalculateMakespan(const std::map<std::string, VMDescription>& taskVM, const std::vector<std::shared_ptr<Task>>& taskOrder);

    void ReduceMakespan(std::map<std::string, VMDescription>& taskVM, 
                        const std::vector<std::shared_ptr<Task>>& taskOrder,
                        double currentMakespan);

    void TasksBundling(const std::map<std::string, VMDescription>& taskVM);

    std::map<std::string, double> CalculateTasksEndTimes(
                        const std::vector<std::shared_ptr<Task>>& taskOrder,
                        const std::map<std::string, VMDescription>& taskVM) const;

    std::map<std::string, std::pair<double, double>> CalculateDeadlines(
                        const std::vector<std::shared_ptr<Task>>& taskOrder,
                        const std::map<std::string, VMDescription>& taskVM) const;

    std::vector<std::vector<LoadVectorEvent>> GetLoadVector(
                        const std::map<std::string, std::pair<double, double>>& deadlines,
                        const std::map<std::string, VMDescription>& taskVM);
};
