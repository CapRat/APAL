//#include <interfaces/IPlugin.hpp>
#include "CatchTools.hpp"
#include "base/PluginBases.hpp"
#include <GlobalData.hpp>
using namespace APAL;
class SimpleExamplePlugin : public LazyPlugin
{
public:
  // Geerbt über LazyPlugin
  virtual void processAudio() override {}
};

REGISTER_PLUGIN(SimpleExamplePlugin);

using namespace APAL;
TEST_CASE("Registration of derived Plugins")
{
  REQUIRE_MESSAGE(GlobalData().getNumberOfRegisteredPlugins() >= 1,
                  "Static Initialisation Failed");
}