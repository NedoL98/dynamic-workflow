#pragma once
#include <vector>
#include <memory>
#include <string>
#include <set>

#include <files/description.h>
#include "argument_parser.h"
namespace Workflow {
    class Task;
    class AbstractGraph {
    protected:
        std::string Name;
    public:
        class GraphAbstractIterator {
        public:
            virtual Task& Get() = 0;
            virtual const Task& Get() const = 0;
            virtual void Next() = 0;
            virtual operator bool() const = 0;
        };

        AbstractGraph()
        {}

        virtual Task& GetTask(int id) = 0; 
        virtual const Task& GetTask(int id) const = 0; 
        virtual Task& GetTaskByName(const std::string& name) = 0; 
        virtual const Task& GetTaskByName(const std::string& name) const = 0;
        virtual GraphAbstractIterator* GetGraphIterator() = 0;
        virtual FileDescription::FileIterator GetReadyFilesIterator() const = 0;
        virtual size_t GetTaskNumber() const = 0;

        virtual bool IsFinished() const = 0;
        virtual void FinishTask(int id) = 0; 
        virtual void StartTransfer(const FileDescription& d) = 0;
        virtual void FinishTransfer(const FileDescription& d) = 0; 
        virtual void AssignTask(int TaskId, int hostId) = 0; 
    };
}
