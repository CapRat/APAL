#include "CatchTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/InfoComponents.hpp>
using namespace APAL;
TEST_CASE("Test InfoComponent Methods")
{
  StaticInfoComponent comp("pluginName",
                           "pluginUri",
                           "pluginDescription",
                           "pluginCopyright",
                           "creatorName",
                           "creatorURL",
                           1234);
  testIInfoComponent(&comp);
}