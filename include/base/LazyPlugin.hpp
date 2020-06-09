#ifndef LAZY_PLUGIN 
#define LAZY_PLUGIN
#include <interfaces/IPlugin.hpp>
#include "SimplePortComponent.hpp"
namespace XPlug  {
    class LazyPlugin :public IPlugin {
    public:
        LazyPlugin();
        LazyPlugin(std::vector<Port> ports);
        // Geerbt über IPlugin
        //virtual void processAudio(const std::vector<Port>& inputs, const std::vector<Port>& outputs) override;
        virtual void init() override;
        virtual void deinit() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual PluginInfo* getPluginInfo() override;
        virtual size_t getParameterCount() override;
        virtual void* getParameter() override;
        virtual void setParameter(void*) override;
        virtual IPortComponent* getPortComponent() override;

    private:
        SimplePortComponent portComponent;
    };
}
#endif //! LAZY_PLUGIN 