#include <gtest/gtest.h>
#include "chimera/logging.h"
#include "emulator.h"

using namespace chimera;
using namespace chimera::test;

//==============================================================================
TEST(Logger, Basics)
{
    //    EXPECT_FALSE(Logger::GetInstance().is_called());

    testing::internal::CaptureStdout();

    EXPECT_TRUE(Emulator::RunVersion());

    std::string output = testing::internal::GetCapturedStdout();

    //    EXPECT_TRUE(Logger::GetInstance().is_called());
}
