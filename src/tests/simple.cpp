#include "common/parse_log.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

TEST(FunctionalTest, NoDependencies) {
    std::string workflow_name = "resources/test_data/no_dependencies.yml";
    std::string vmlist_name = "resources/test_data/single_vm.yml";
    std::stringstream commandBuilder;
    commandBuilder << "build/dynamic-workflow-debug " <<
            " --workflow " << workflow_name <<
            " --vm_list "  << vmlist_name <<
            " --deadline " << "100";
    std::string command = commandBuilder.str();
    ASSERT_FALSE(system(command.c_str()));
    RunSpec runResult = ParseLogFile("event.log");
    ASSERT_EQ(runResult.Time, 50);
    ASSERT_EQ(runResult.Cost, 5000);
}


TEST(FunctionalTest, SimpleChain) {
    std::string workflow_name = "resources/test_data/simple_chain.yml";
    std::string vmlist_name = "resources/test_data/single_vm.yml";
    std::stringstream commandBuilder;
    commandBuilder << "build/dynamic-workflow-debug " <<
            " --workflow " << workflow_name <<
            " --vm_list "  << vmlist_name <<
            " --scheduling_algorithm mao" <<
            " --deadline " << "100";
    std::string command = commandBuilder.str();
    ASSERT_FALSE(system(command.c_str()));
    RunSpec runResult = ParseLogFile("event.log");
    double totalExecuting = 0;
    for (const auto& vmSpec : runResult.VMRuns) {
        totalExecuting += vmSpec.ExecutingTime;
        std::cout << vmSpec.StartTime << ' ' << vmSpec.FinishTime << ' ' << vmSpec.ExecutingTime << '\n';
    }
    ASSERT_EQ(totalExecuting, 65);
}

TEST(FunctionalTest, VMStartupLag) {
    std::string workflow_name = "resources/test_data/no_dependencies.yml";
    std::string vmlist_name = "resources/test_data/laggy_vm.yml";
    std::stringstream commandBuilder;
    commandBuilder << "build/dynamic-workflow-debug " <<
            " --workflow " << workflow_name <<
            " --vm_list "  << vmlist_name <<
            " --deadline " << "100";
    std::string command = commandBuilder.str();
    ASSERT_FALSE(system(command.c_str()));
    RunSpec runResult = ParseLogFile("event.log");
    ASSERT_EQ(runResult.Time, 51);
    // ASSERT_EQ(runResult.Cost, 5000);
}

TEST(FunctionalTest, MultipleTransfers) {
    std::string workflow_name = "resources/test_data/multiple_transfers.yml";
    std::string vmlist_name = "resources/test_data/single_vm.yml";
    std::string platform_name = "resources/test_data/multiple_transfers_platform.xml";
    std::stringstream commandBuilder;
    commandBuilder << "build/dynamic-workflow-debug " <<
            " --workflow " << workflow_name <<
            " --vm_list "  << vmlist_name <<
            " --platform " << platform_name <<
            " --scheduling_algorithm mao" <<
            " --deadline " << "100";
    std::string command = commandBuilder.str();
    ASSERT_FALSE(system(command.c_str()));

    RunSpec runResult = ParseLogFile("event.log");
    ASSERT_GE(runResult.Time, 14300.0 / 1000);
}

TEST(FunctionalTest, VMOrder) {
    {
        std::string workflow_name = "resources/test_data/no_dependencies.yml";
        std::string vmlist_name = "resources/test_data/vm_order_straight.yml";
        std::stringstream commandBuilder;
        commandBuilder << "build/dynamic-workflow-debug " <<
                " --workflow " << workflow_name <<
                " --vm_list "  << vmlist_name <<
                " --deadline " << "100";
        std::string command = commandBuilder.str();
        ASSERT_FALSE(system(command.c_str()));
    }
    RunSpec straightRunResult = ParseLogFile("event.log");
    {
        std::string workflow_name = "resources/test_data/no_dependencies.yml";
        std::string vmlist_name = "resources/test_data/vm_order_reversed.yml";
        std::stringstream commandBuilder;
        commandBuilder << "build/dynamic-workflow-debug " <<
                " --workflow " << workflow_name <<
                " --vm_list "  << vmlist_name <<
                " --deadline " << "100";
        std::string command = commandBuilder.str();
        ASSERT_FALSE(system(command.c_str()));
    }
    RunSpec revRunResult = ParseLogFile("event.log");
    ASSERT_EQ(straightRunResult.Time, revRunResult.Time);
    ASSERT_EQ(straightRunResult.Cost, revRunResult.Cost);
    ASSERT_EQ(straightRunResult.Cost, 5000);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

