#include "VST3Initialisation.hpp"
using namespace XPlug;
using namespace Steinberg;

/*
ProcessorHandle::ProcessorHandle()
{
	this->context.state = 0;	///< a combination of the values from \ref StatesAndFlags
	this->context.sampleRate = 0;	///< current sample rate (always valid)
	this->context.projectTimeSamples = 0;///< project time in samples (always valid)
	this->context.systemTime = 0;///< system time in nanoseconds (optional)
	this->context.continousTimeSamples = 0;///< project time, without loop (optional)
	this->context.projectTimeMusic = 0;///< musical position in quarter notes (1.0 equals 1 quarter note)
	this->context.barPositionMusic = 0;///< last bar start position, in quarter notes
	this->context.cycleStartMusic = 0;///< cycle start in quarter notes
	this->context.cycleEndMusic = 0;///< cycle end in quarter notes
	this->context.tempo = 0;///< tempo in BPM (Beats Per Minute)
	this->context.timeSigDenominator = 0;///< time signature numerator (e.g. 3 for 3/4)
	this->context.timeSigDenominator = 0;///< time signature denominator (e.g. 4 for 3/4)
	this->context.chord = { 0,0,0 };	///< musical info
	this->context.smpteOffsetSubframes = 0;	///< SMPTE (sync) offset in subframes (1/80 of frame)
	this->context.frameRate = { 0,0 };	///< frame rate
	this->context.samplesToNextClock = 0;///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest)
}*/



TEST_CASE("Initialise VST3 Functions", "[symbol][vst3]") {
	//Vst3Handle();
	//VST3Module testVar;
	GetCachedModule();
}





VST3Module::VST3Module()
{
	this->pluginSymbols.init();
	this->pluginSymbols.runInitFncLampda();
	this->factoryHandler.init(this->pluginSymbols);
}

VST3Module::~VST3Module()
{
	factoryHandler.deinit();
	this->pluginSymbols.runExitFncLampda();
	pluginSymbols.deinit();
}

Steinberg::Vst::IAudioProcessor* VST3Module::getProcessor()
{
	Steinberg::Vst::IAudioProcessor* proc;
	this->factoryHandler.processorComponent->queryInterface(Steinberg::Vst::IAudioProcessor::iid, (void**)&proc);
	return proc;
}

Steinberg::Vst::IEditController* VST3Module::getController()
{
	return  this->factoryHandler.editController;
}

Steinberg::Vst::IComponent* VST3Module::getComponent()
{
	return  this->factoryHandler.processorComponent;
}

VST3Module::VST3Module(bool initNothing)
{
}


void test_function_loading()
{
	//Empty module. Cleanup will be called anyways.
	VST3Module emptyModule(true);
	emptyModule.pluginSymbols.init();
	SECTION("Running initialisation and loadingfunctions") {
		REQUIRE_MESSAGE(emptyModule.pluginSymbols.runInitFncLampda(), "Initialisation function failed, when executed");
		REQUIRE_MESSAGE(emptyModule.pluginSymbols.plugLoadFunction() != nullptr, "No Factory returned, when callin GetPluginFactory");
		REQUIRE_MESSAGE(emptyModule.pluginSymbols.runExitFncLampda(), "Deinitialisation function failed, when executed");
	}
}
VST3Module& GetCachedModule()
{
	static VST3Module* mod = new VST3Module();
	return *mod;
	// TODO: hier return-Anweisung eingeben
}
TEST_CASE("Load Library and Required Functions", "[symbol][vst3]")
{
	test_function_loading();
}



void VST3Module::pluginSymbols_type::init()
{
	STATUS_TASK("loading the vst-library");
	GetError();
	this->pluginLibrary = LoadLib(pluginPath.c_str());
	REQUIRE_MESSAGE((this->pluginLibrary != nullptr), GetErrorStr());
	STATUS_RESULT("successfully loaded!");

	STATUS_TASK("Loading the  required functions from vst3 lib");
	this->plugLoadFunction = LoadFunc<GetFactoryProc >(this->pluginLibrary, "GetPluginFactory");
	REQUIRE_MESSAGE((this->plugLoadFunction != nullptr), GetErrorStr());
#if defined(WIN32)
	this->initfnc = LoadFuncRaw(this->pluginLibrary, "InitDll");
	this->exitfnc = LoadFuncRaw(this->pluginLibrary, "ExitDll");
	this->runInitFncLampda = [this]()->bool {return ((bool (*)())this->initfnc)(); };
	this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#elif defined(__APPLE__)
	this->initfnc = LoadFuncRaw(this->pluginLibrary, "bundleEntry");
	this->exitfnc = LoadFuncRaw(this->pluginLibrary, "bundleExit");
	this->runInitFncLampda = [this]()->bool {return ((bool (*)(CFBundleRef))this->initfnc)(); };
	this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#elif defined(__linux__)
	this->initfnc = LoadFuncRaw(this->pluginLibrary, "ModuleEntry");
	this->exitfnc = LoadFuncRaw(this->pluginLibrary, "ModuleExit");
	this->runInitFncLampda = [this]()->bool {return ((bool (*)(void*))this->initfnc)((void*)pluginLibrary); };
	this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#else 
	assert("Warning, cant determinate Sytem. No Initfunction(like InitDll) is executed nor testeted")
#endif //!WIN32
		REQUIRE_MESSAGE((this->initfnc != nullptr && this->exitfnc != nullptr), GetErrorStr());
	STATUS_RESULT("succsessfully loaded vst3 functions like GetPluginsFactory and init and exit functions.");
}

void VST3Module::pluginSymbols_type::deinit()
{
	UnloadLib(this->pluginLibrary);
}

void VST3Module::FactoryHandler_Type::getClassInfo(size_t index, Steinberg::PClassInfoW * info)
{

	if (factory3 != nullptr) {
		this->factory3->getClassInfoUnicode(index,info);
	}
	else if (factory2 != nullptr) {
		Steinberg::PClassInfo2 ci2;
		this->factory2->getClassInfo2(index, &ci2);
		info->fromAscii(ci2);
	}
	else {
		Steinberg::PClassInfo ci;
		this->factory->getClassInfo(index, &ci);
		PFactoryInfo facInfo;
		REQUIRE_STRICT_MESSAGE((this->factory->getFactoryInfo(&facInfo) == kResultOk), "Cant get FactoryInfo from factory");
		Steinberg::PClassInfo2 ci2(ci.cid, ci.cardinality, ci.category, ci.name, 0, NULL, facInfo.vendor, "0.0.0.0", "VST 3.0.0");
		info->fromAscii(ci2);
	}
}

void VST3Module::FactoryHandler_Type::init(pluginSymbols_type& symbols)
{
	this->factory = symbols.plugLoadFunction();
	REQUIRE_MESSAGE((this->factory != nullptr), "Error while loading factory. Factory is NULL");
	REQUIRE_MESSAGE((this->factory->countClasses() >= 2), "Error There are less then 2 classes in Factory. There must be at least one IAudioProcessor and one IEditController class.");
	PFactoryInfo info;
	REQUIRE_STRICT_MESSAGE((this->factory->getFactoryInfo(&info) == kResultOk), "Cant get FactoryInfo from factory");
	this->factory->queryInterface(IPluginFactory3::iid, (void**)&factory3);
	this->factory->queryInterface(IPluginFactory2::iid, (void**)&factory2);
	for (int i = 0; i < this->factory->countClasses(); i++) {
		PClassInfoW ci3;
		this->getClassInfo(i,&ci3);
		if (strcmp(ci3.category, kVstAudioEffectClass) == 0) {//Class is an Audio Effect Processor
		
			auto res = this->factory->createInstance(ci3.cid, Vst::IComponent::iid, (void**)&processorComponent);
			REQUIRE_MESSAGE((processorComponent && (res == kResultOk)), "Error, could not create IComponentobject successfully");
			REQUIRE_MESSAGE((processorComponent->initialize(nullptr) == kResultOk), "Error while initialising Component " << ci3.name);
			TUID controllerCID;
			REQUIRE_MESSAGE(processorComponent->getControllerClassId(controllerCID)==kResultTrue, "Error,while getting ControllerID from IComponent.");
			auto controllerFUID =FUID::fromTUID(controllerCID);
			REQUIRE_MESSAGE(controllerFUID.isValid(), "Error, ControllerID from IComponent is not valid.");
			res= this->factory->createInstance(controllerCID, Vst::IEditController::iid, (void**)&editController);
			REQUIRE_MESSAGE((editController && res == kResultOk), "Error while creating EditController from Factory.")
			REQUIRE_MESSAGE(editController->initialize(nullptr)==kResultOk, "Error while initializing EditController");
		}
		else {
			continue;
		}
	}

}

void VST3Module::FactoryHandler_Type::deinit()
{
	if (editController != nullptr)
		REQUIRE_MESSAGE(editController->terminate() == kResultOk, "Error while terminating IEditController implementation.");
	if (processorComponent != nullptr)
		REQUIRE_MESSAGE(processorComponent->terminate() == kResultOk, "Error while terminating IComponent implementation.");
}

