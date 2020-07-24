#ifndef LAZY_PLUGIN 
#define LAZY_PLUGIN
#include <memory>
#include <interfaces/IPlugin.hpp>
#include "PortComponents.hpp"
#include "FeatureComponents.hpp"
#include "InfoComponents.hpp"
namespace XPlug  {
    /**
     * @brief Modular Plugin, which holds all required components. The componenttypes must be specified. 
     * If the default constructor is used, every component is NULL and must be initialized in the implementation Plugin.
     * @tparam TIInfoComponent InfoComponent, used by this Plugin.
     * @tparam TIPortComponent PortComponent, used by this Plugin.
     * @tparam TIFeatureComponent FeatureComponent, used by this Plugin.
    */
    template<typename TIInfoComponent, typename TIPortComponent,typename TIFeatureComponent>
    class ModularPlugin :public IPlugin {
    protected:
        std::unique_ptr<TIInfoComponent> infoComponent = nullptr;
        std::unique_ptr <TIPortComponent> portComponent = nullptr;
        std::unique_ptr <TIFeatureComponent> featureComponent = nullptr;
    public:
        ModularPlugin() = default;
        ModularPlugin(std::unique_ptr<TIInfoComponent> infoComponent, std::unique_ptr <TIPortComponent> portComponent, std::unique_ptr <TIFeatureComponent> featureComponent) {
            this->infoComponent = std::move(infoComponent);
            this->portComponent = std::move(portComponent);
            this->featureComponent = std::move(featureComponent);
        }
        virtual IInfoComponent* getInfoComponent() override { return infoComponent.get(); }
        virtual IPortComponent* getPortComponent() override { return portComponent.get(); }
        virtual IFeatureComponent* getFeatureComponent() override { return featureComponent.get(); }
    };

    /**
     * @brief LazyPlugin, which just implements all Methods, except Process. Also it uses some Components, which match the Lazy Approach.
     */
    class LazyPlugin :public ModularPlugin< StaticInfoComponent, DynamicPortComponent, AutomaticFeatureComponent>{
    public:
        LazyPlugin();
        LazyPlugin(std::vector<std::unique_ptr<IPort>> ports);

        // Geerbt �ber IPlugin
        virtual void init() override;
        virtual void deinit() override;
        virtual void activate() override;
        virtual void deactivate() override;
    };

}
#endif //! LAZY_PLUGIN 