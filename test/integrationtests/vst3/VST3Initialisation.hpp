#ifndef SYMBOL_LOADING_HPP
#define SYMBOL_LOADING_HPP
#include <functional>
#include <vector>
#include "tools/LibLoading.hpp"
#include "../XValidate.hpp"

#include <vst/ivstaudioprocessor.h>
#include <vst/ivsteditcontroller.h>
#include <vst/ivstprocesscontext.h>

void test_function_loading();






class VST3Module {
	friend void test_function_loading();
public:
	VST3Module();
	~VST3Module();
	Steinberg::Vst::IAudioProcessor* getProcessor();
	Steinberg::Vst::IEditController* getController();
	Steinberg::Vst::IComponent* getComponent();

private:
	/**
	 * @brief Constructor, which should instantiate nothing. Useful for single Tests. Dont use this where else.
	 * @param initNothing  Dummy parameter for overloading. 
	 */
	VST3Module(bool initNothing);
	struct pluginSymbols_type{
		XPlug::library_t pluginLibrary; // The pluginlibrary
		GetFactoryProc plugLoadFunction; // FactoryLoad Function
		void* initfnc; // functionpointer to init function
		void* exitfnc; // functionpointer to init function
		std::function<bool()>runInitFncLampda; // Function to call init function crossplattform. Returns Value from InitDll/InitModule/bundleEntry
		std::function<bool()>runExitFncLampda; // Function to call exit function crossplattform.Returns Value from  ExitDll/ExitModule/bundleExit
		/**
		 * @brief loads and resolves symbols from current Plugin.
		 */
		void init(); 
		/**
		 * @brief Unloads library and cleanup symbols.
	 	 */
		void deinit();
	}pluginSymbols;

	
	struct FactoryHandler_Type {
		Steinberg::Vst::IComponent* processorComponent = nullptr;
		Steinberg::Vst::IEditController* editController = nullptr;
		Steinberg::IPluginFactory* factory=nullptr;
		Steinberg::IPluginFactory2* factory2 = nullptr;
		Steinberg::IPluginFactory3* factory3 = nullptr;

		void getClassInfo(size_t index, Steinberg::PClassInfoW*info);
		void init(pluginSymbols_type& symbols);
		void deinit();
	}factoryHandler;

	//std::vector<ProcessorHandle> processorHandles;

};

/**
 * @brief Returns the Cached Module, so that in total just one Module is created.
 * @return 
*/
VST3Module& GetCachedModule();






#endif //! SYMBOL_LOADING_HPP