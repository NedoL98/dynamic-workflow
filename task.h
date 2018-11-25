#include <simgrid/s4u.hpp>

class Task {
public:
    Task() = default;
    
    Task(long long flops, int cores, int ram);

    long long GetFlops();
    int GetCores();
    int GetRam();

    static void DoExecute(long long flops);
    void Execute(simgrid::s4u::Host* host);

private:
    long long Flops;
    int Cores;
    int Ram;
};