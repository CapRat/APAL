#ifndef LAZY_PLUGIN 
#define LAZY_PLUGIN
#include <interfaces/IPlugin.hpp>
#include "DynamicPortComponent.hpp"
namespace XPlug  {
    class LazyPlugin :public IPlugin {
    public:
        LazyPlugin();
        LazyPlugin(std::vector<std::unique_ptr<IPort>> ports);

        // Geerbt über IPlugin
        virtual void init() override;
        virtual void deinit() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual PluginInfo* getPluginInfo() override;
        virtual size_t getParameterCount() override;
        virtual void* getParameter() override;
        virtual void setParameter(void*) override;
        virtual IPortComponent* getPortComponent() override;

    protected:
        DynamicPortComponent portComponent;
       
    };

}
#endif //! LAZY_PLUGIN 