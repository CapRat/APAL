#ifndef LAZY_PLUGIN 
#define LAZY_PLUGIN
#include <interfaces/IPlugin.hpp>
#include "PortComponents.hpp"
#include "FeatureComponents.hpp"
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
        virtual IPortComponent* getPortComponent() override;
        virtual IFeatureComponent* getFeatureComponent() override;
    protected:
        DynamicPortComponent portComponent;
        AutomaticFeatureComponent featureComp;
        PluginInfo inf;
    };

}
#endif //! LAZY_PLUGIN 