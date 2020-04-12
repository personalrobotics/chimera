#include <gtest/gtest.h>
#include "emulator.h"

using namespace chimera;
using namespace chimera::test;

//==============================================================================
TEST(Emulator, GeneralOptions)
{
    testing::internal::CaptureStdout();

    // EXPECT_EXIT is used to continue running the rest of the tests. This is
    // because ClangTool exits right after it prints help. Note that this makes
    // the breakpoints don't work in executing the test s in EXPECT_EXIT.
    EXPECT_EXIT(Emulator::RunHelp(), ::testing::ExitedWithCode(0), ".*");
    EXPECT_EXIT(Emulator::Run("--help"), ::testing::ExitedWithCode(0), ".*");
    EXPECT_EXIT(Emulator::RunHelpList(), ::testing::ExitedWithCode(0), ".*");
    EXPECT_EXIT(Emulator::Run("--help-list"), ::testing::ExitedWithCode(0),
                ".*");

    EXPECT_TRUE(Emulator::RunVersion());
    EXPECT_TRUE(Emulator::Run("--version"));

    std::string output = testing::internal::GetCapturedStdout();
}

//==============================================================================
TEST(Emulator, 01_Function)
{
    Emulator e;
    e.SetSource("01_function/function.h");
    e.SetConfigurationFile("01_function/function_pybind11.yaml");
    e.SetBinding("pybind11");

    EXPECT_TRUE(e.Run());
}

//==============================================================================
TEST(Emulator, 02_Class)
{
    Emulator e;
    e.SetSource("02_class/class.h");
    e.SetConfigurationFile("02_class/class.yaml");
    e.SetBinding("pybind11");

    EXPECT_TRUE(e.Run());
}

//==============================================================================
TEST(Emulator, 04_Enumeration)
{
    Emulator e;
    e.SetSource("04_enumeration/enumeration.h");
    e.SetConfigurationFile("04_enumeration/enumeration.yaml");
    e.SetBinding("pybind11");

    EXPECT_TRUE(e.Run());
}

//==============================================================================
TEST(Emulator, 20_Eigen)
{
    Emulator e;
    e.SetSource("20_eigen/eigen.h");
    e.SetConfigurationFile("20_eigen/eigen_pybind11.yaml");
    e.SetBinding("pybind11");

    EXPECT_TRUE(e.Run());
}

//==============================================================================
TEST(Emulator, issue228)
{
    Emulator e;
    e.SetSource(
        "regression/issue228_template_type_alias/"
        "issue228_template_type_alias.h");
    e.SetConfigurationFile(
        "regression/issue228_template_type_alias/"
        "issue228_template_type_alias_pybind11.yaml");
    e.SetBinding("pybind11");

    EXPECT_TRUE(e.Run());
}
