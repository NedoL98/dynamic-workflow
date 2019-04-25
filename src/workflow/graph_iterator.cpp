#include <workflow/graph.h>

namespace Workflow {
    Graph::GraphIterator::GraphIterator(std::vector<std::unique_ptr<Task>>& v, int pos)
        : data(v)
        , i(pos)
        {}

    Task& Graph::GraphIterator::Get() {
        return *data[i];
    }
        
    const Task& Graph::GraphIterator::Get() const {
        return *data[i];
    }
    void Graph::GraphIterator::Next() {
        i++;
    }

    Graph::GraphIterator::operator bool() const {
        return 0 <= i && i < static_cast<int>(data.size());
    }
}
