#ifndef INFO_COMPONENTS_HPP
#define INFO_COMPONENTS_HPP
#include <interfaces/IInfoComponent.hpp>
namespace XPlug {

/**
 * @brief  currently only implementation of the IInfoComponent. Stores infos in
 * variables and satisfies the Interface.
 */
class StaticInfoComponent : public IInfoComponent
{
public:
  std::string pluginName;
  std::string pluginUri;
  std::string pluginDescription;
  std::string pluginCopyright;
  std::string creatorName;
  std::string creatorURL;
  StaticInfoComponent();
  StaticInfoComponent(std::string pluginName,
                      std::string pluginUri,
                      std::string pluginDescription,
                      std::string pluginCopyright,
                      std::string creatorName,
                      std::string creatorURL);

  virtual std::string_view getPluginName() override;
  virtual std::string_view getPluginURI() override;
  virtual std::string_view getPluginDescription() override;
  virtual std::string_view getPluginCopyright() override;
  virtual std::string_view getCreatorName() override;
  virtual std::string_view getCreatorURL() override;
};
}
#endif //! INFO_COMPONENTS_HPP