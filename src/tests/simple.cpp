#include "common/parse_log.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <sstream>

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

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

