#include "VST3Initialisation.hpp"
#include <ivsteditcontroller.h>
#include <ivstcomponent.h>

using namespace Steinberg;

Vst::IComponent* processorComponent;
Vst::IEditController* editController;
IPluginFactory* factory;
TEST_CASE("Just process the Stuff.", "[processing][vst3]") {
	auto module = GetCachedModule();
	Steinberg::Vst::ProcessData data;

	//module.getProcessor()->process()
}