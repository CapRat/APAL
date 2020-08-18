#ifndef LAZY_PLUGIN
#define LAZY_PLUGIN
#include "FeatureComponents.hpp"
#include "InfoComponents.hpp"
#include "PortComponents.hpp"
#include <interfaces/IPlugin.hpp>
#include <memory>
namespace APAL {
/**
 * @brief Modular Plugin, which holds all required components. The
 * componenttypes must be specified. If the default constructor is used, every
 * component is NULL and must be initialized in the implementation Plugin.
 * @tparam TIInfoComponent InfoComponent, used by this Plugin.
 * @tparam TIPortComponent PortComponent, used by this Plugin.
 * @tparam TIFeatureComponent FeatureComponent, used by this Plugin.
 */
template<typename TIInfoComponent,
         typename TIPortComponent,
         typename TIFeatureComponent>
class ModularPlugin : public IPlugin
{
protected:
  std::unique_ptr<TIInfoComponent> infoComponent = nullptr;
  std::unique_ptr<TIPortComponent> portComponent = nullptr;
  std::unique_ptr<TIFeatureComponent> featureComponent = nullptr;

public:
  ModularPlugin() = default;
  ModularPlugin(std::unique_ptr<TIInfoComponent> _infoComponent,
                std::unique_ptr<TIPortComponent> _portComponent,
                std::unique_ptr<TIFeatureComponent> _featureComponent)
  {
    this->infoComponent = std::move(_infoComponent);
    this->portComponent = std::move(_portComponent);
    this->featureComponent = std::move(_featureComponent);
  }
  virtual IInfoComponent* getInfoComponent() override
  {
    return infoComponent.get();
  }
  virtual IPortComponent* getPortComponent() override
  {
    return portComponent.get();
  }
  virtual IFeatureComponent* getFeatureComponent() override
  {
    return featureComponent.get();
  }
};

/**
 * @brief LazyPlugin, which just implements all methods, except process. Also it
 * uses some components, which match the lazy approach.
 */
class LazyPlugin
  : public ModularPlugin<StaticInfoComponent,
                         DynamicPortComponent,
                         AutomaticFeatureComponent>
{
public:
  LazyPlugin();
  LazyPlugin(std::vector<std::unique_ptr<IPort>> _ports);

  // Geerbt über IPlugin
  virtual void init() override;
  virtual void deinit() override;
  virtual void activate() override;
  virtual void deactivate() override;
};

}
#endif //! LAZY_PLUGIN