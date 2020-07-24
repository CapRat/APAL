#include "UnitTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/InfoComponents.hpp>
using namespace XPlug;
TEST_CASE("Test InfoComponent Methods") {
    StaticInfoComponent comp("pluginName", "pluginUri","pluginDescription", "pluginCopyright",  "creatorName", "creatorURL");
    testIInfoComponent(&comp);
}