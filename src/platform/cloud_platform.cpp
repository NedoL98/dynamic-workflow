#include "platform/cloud_platform.h"
#include <simgrid/s4u.hpp>
#include "spec.h"
#include <vector>

XBT_LOG_NEW_DEFAULT_CATEGORY(platform_cloud_platform, "cloud platform log");

namespace {
    void DoExecute(long long flops, const std::function<void(int, void*)>& onExit, void* arg, double lag = 0) {
        simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

        double timeStart = simgrid::s4u::Engine::get_clock();
        simgrid::s4u::this_actor::sleep_for(lag);
        host->execute(flops);
        double timeFinish = simgrid::s4u::Engine::get_clock();


        XBT_INFO("%s: actor %s executed %g seconds", host->get_cname(),
                simgrid::s4u::this_actor::get_cname(), timeFinish - timeStart);
        onExit(0, arg);
        simgrid::s4u::this_actor::exit();
    }

    void DoTransfer(int i, const std::function<void(TransferSpec*)>& onTransferSuccess) {
        XBT_INFO("Transfer action started");
        auto coordinator = simgrid::s4u::Mailbox::by_name("coordinate_" + std::to_string(i));
        bool isKilled = false;
        TransferSpec* buffer = nullptr;
        void *dataBuffer = nullptr;
        std::vector<simgrid::s4u::CommPtr> activeTransfers;
        std::map<simgrid::s4u::CommPtr, TransferSpec*> transferBuffers;
        while (!isKilled) {
            int finishedTransfer = 0;
            while ((finishedTransfer = simgrid::s4u::Comm::test_any(&activeTransfers)) != -1) {
                XBT_INFO("Downloading is finished on host %d", i);
                auto finishedSpec = transferBuffers[activeTransfers[finishedTransfer]];
                onTransferSuccess(finishedSpec);
                std::swap(activeTransfers[finishedTransfer], activeTransfers.back());
                activeTransfers.pop_back();
            }

            buffer = nullptr;
            if (!coordinator->empty()) {
                buffer = (TransferSpec *)coordinator->get();
            } else {
                simgrid::s4u::this_actor::sleep_for(1);
            }
            if (!buffer) { // exit by timeout
                continue;
            } else if (buffer->Receiver == -1) { // Terminating messages
                isKilled = true;
                XBT_INFO("Transfer manager %d is killed", i);
            } else if (buffer->Sender == buffer->Receiver) { // File is already where it should be
                onTransferSuccess(buffer);
                coordinator->put(buffer, 0);
            } else if (buffer->Receiver == i) { // We are receiving file
                XBT_INFO("Downloading is started on host %d", i);
                std::string name = "transfer_" + std::to_string(buffer->Receiver);
                auto sender = simgrid::s4u::Mailbox::by_name(name);
                activeTransfers.push_back(sender->get_init());
                transferBuffers[activeTransfers.back()] = nullptr;
                activeTransfers.back()->set_dst_data((void**)&transferBuffers[activeTransfers.back()], sizeof(transferBuffers[activeTransfers.back()]));
                activeTransfers.back()->start();
                coordinator->put(&sender, 0);
            } else { // We are sending file
                std::string name = "transfer_" + std::to_string(buffer->Receiver);
                auto receiver = simgrid::s4u::Mailbox::by_name(name);
                TransferSpec* bufferSpec = new TransferSpec(*buffer);
                activeTransfers.push_back(receiver->put_init(bufferSpec, buffer->Size));
                transferBuffers[activeTransfers.back()] = bufferSpec;
                activeTransfers.back()->start();
                coordinator->put(&receiver, 0);
            }
        }
        for (const auto& comm : activeTransfers) {
            comm->wait();
        }
    }
}

CloudPlatform::~CloudPlatform() {
    TransferSpec spec({-1, -1, -1, -1, -1});
    for (const auto& [id, ptr] : VirtualMachines) {
        auto senderMailbox = simgrid::s4u::Mailbox::by_name("coordinate_" + std::to_string(id));
        senderMailbox->put((void*)&spec, 0);
        Mailers[id]->join();
    }
}
        
bool CloudPlatform::CreateVM(int hostId, const VMDescription& s, int id) {
    if (VirtualMachines.count(id)) {
        return false;
    }
    if (!HostsList[hostId].CreateVM(s, id)) {
        return false;
    }
    VirtualMachines[id] = HostsList[hostId].VirtualMachines[id];
    VirtualMachineSpecs[id] = s.GetSpec();
    auto mailer = simgrid::s4u::Actor::create("transfer_" + std::to_string(id),
                                            VirtualMachines[id],
                                            DoTransfer, id, OnTransferFinish);
    Mailers[id] = mailer;
    return true;
}

bool CloudPlatform::CheckTask(int vmId, const TaskSpec& requirements) {
    if (!VirtualMachines.count(vmId)) {
        return false; 
    }
    if (VirtualMachineSpecs[vmId].Cores < requirements.GetCores() || 
        VirtualMachineSpecs[vmId].Memory < requirements.GetMemory()) {
        return false;
    }
    return true;
}

simgrid::s4u::ActorPtr CloudPlatform::AssignTask(int vmId, const TaskSpec& requirements, const std::function<void(int, void*)>& onExit, void* args) {
    static int i = 0;
    if (!CheckTask(vmId, requirements)) {
        XBT_INFO("Does not satisfy requirements, doesn't run!");
        return nullptr;
    }
    simgrid::s4u::ActorPtr result = nullptr;
    if (VirtualMachines[vmId]->get_state() == simgrid::s4u::VirtualMachine::state::CREATED) {
        double timeStart = simgrid::s4u::Engine::get_clock();
        double startupCost = (MinStartupTime + MaxStartupTime) / 2;
        VirtualMachines[vmId]->start();
        XBT_INFO("Turning on vm #%d in %.6g", vmId, timeStart);
        result = simgrid::s4u::Actor::create("compute" + std::to_string(i),
                                            VirtualMachines[vmId],
                                            DoExecute,
                                            requirements.GetSize(),
                                            onExit,
                                            args, startupCost);
        XBT_INFO("VM %d was sleeping and started at %.6g", vmId, timeStart);
    } else {
        result = simgrid::s4u::Actor::create("compute" + std::to_string(i),
                                            VirtualMachines[vmId],
                                            DoExecute,
                                            requirements.GetSize(),
                                            onExit,
                                            args, 0);
    }
        
    i++;
    VirtualMachineSpecs[vmId].Cores -= requirements.GetCores();
    VirtualMachineSpecs[vmId].Memory -= requirements.GetMemory();
    return result;
}

int CloudPlatform::GetEmptyHost(const ComputeSpec& s) {
    for (const auto& host : HostsList) {
        if (host.VirtualMachines.empty() && host.Spec.Memory >= s.Memory && host.Spec.Cores >= s.Cores) {
            return host.Id;
        }
    }
    return -1;
}

void CloudPlatform::FinishTask(int vmId, const TaskSpec& requirements) {
    VirtualMachineSpecs[vmId].Cores += requirements.GetCores();
    VirtualMachineSpecs[vmId].Memory += requirements.GetMemory();
}

void CloudPlatform::StartTransfer(const TransferSpec& file) {
    XBT_INFO("transferSpec: %d %d %lld", file.Sender, file.Receiver, file.Size);
    auto senderMailbox = simgrid::s4u::Mailbox::by_name("coordinate_" + std::to_string(file.Sender));
    auto receiverMailbox = simgrid::s4u::Mailbox::by_name("coordinate_" + std::to_string(file.Receiver));
    void *reply = this;
    if (file.Sender == file.Receiver) {
        simgrid::s4u::CommPtr senderMessage = senderMailbox->put_async((void*)&file, 0);
        senderMessage->wait();
        reply = senderMailbox->get();
    } else {
        simgrid::s4u::CommPtr senderMessage = senderMailbox->put_async((void*)&file, 0);
        simgrid::s4u::CommPtr receiverMessage = receiverMailbox->put_async((void*)&file, 0);
        senderMessage->wait();
        receiverMessage->wait();
        reply = senderMailbox->get();
        reply = receiverMailbox->get();
    }
    (void)reply;
}

