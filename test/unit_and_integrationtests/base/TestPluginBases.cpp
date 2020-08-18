#include "CatchTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/PluginBases.hpp>
using namespace APAL;
class LazyPluginTestClass : public LazyPlugin
{
public:
  // Geerbt über LazyPlugin
  virtual void processAudio() override {}
};

TEST_CASE("Test PluginBases Methods")
{
  LazyPluginTestClass lPlug;
  testIPlugin(&lPlug);
  SUCCEED("Valid IPlugin bases");
}