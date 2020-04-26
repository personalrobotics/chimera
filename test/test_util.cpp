#include <gtest/gtest.h>
#include "chimera/util.h"

using namespace chimera;

//==============================================================================
TEST(Util, StringCast)
{
    // Test toUpper
    EXPECT_EQ(util::toUpper("Illusory_or_Impossible"),
              "ILLUSORY_OR_IMPOSSIBLE");
    EXPECT_EQ(util::toUpper("Illusory_or_Impossible", -1),
              "ILLUSORY_OR_IMPOSSIBLE");
    EXPECT_EQ(util::toUpper("Illusory_or_Impossible", 0),
              "Illusory_or_Impossible");
    EXPECT_EQ(util::toUpper("Illusory_or_Impossible", 2),
              "ILlusory_or_Impossible");

    // Test toLower
    EXPECT_EQ(util::toLower("Illusory_or_Impossible"),
              "illusory_or_impossible");
    EXPECT_EQ(util::toLower("Illusory_or_Impossible", -1),
              "illusory_or_impossible");
    EXPECT_EQ(util::toLower("Illusory_or_Impossible", 0),
              "Illusory_or_Impossible");
    EXPECT_EQ(util::toLower("Illusory_or_Impossible", 2),
              "illusory_or_Impossible");

    // Test toPascal
    EXPECT_EQ(util::toPascal("illusory_or_impossible"), "IllusoryOrImpossible");
    EXPECT_EQ(util::toPascal("illusoryOrImpossible"), "IllusoryOrImpossible");
    EXPECT_EQ(util::toPascal("IllusoryOrImpossible"), "IllusoryOrImpossible");
    EXPECT_EQ(util::toPascal("illusory_or__impossible"),
              "IllusoryOrImpossible");
    EXPECT_EQ(util::toPascal("illusory_or_impossible___"),
              "IllusoryOrImpossible");
    EXPECT_EQ(util::toPascal("____illusory_or_impossible"),
              "IllusoryOrImpossible");

    // Test toCamel
    EXPECT_EQ(util::toCamel("illusory_or_impossible"), "illusoryOrImpossible");
    EXPECT_EQ(util::toCamel("illusoryOrImpossible"), "illusoryOrImpossible");
    EXPECT_EQ(util::toCamel("IllusoryOrImpossible"), "illusoryOrImpossible");
    EXPECT_EQ(util::toCamel("illusory_or__impossible"), "illusoryOrImpossible");
    EXPECT_EQ(util::toCamel("illusory_or_impossible___"),
              "illusoryOrImpossible");
    EXPECT_EQ(util::toCamel("____illusory_or_impossible"),
              "illusoryOrImpossible");
}
