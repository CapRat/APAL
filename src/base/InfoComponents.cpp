#include <base/InfoComponents.hpp>
using namespace APAL;
APAL::StaticInfoComponent::StaticInfoComponent()
{
  this->pluginName = "";
  this->pluginUri = "";
  this->pluginDescription = "";
  this->pluginCopyright = "";
  this->creatorName = "";
  this->creatorURL = "";
  this->id = 0;
}
APAL::StaticInfoComponent::StaticInfoComponent(std::string _pluginName,
                                                std::string _pluginUri,
                                                std::string _pluginDescription,
                                                std::string _pluginCopyright,
                                                std::string _creatorName,
                                                std::string _creatorURL,
                                                int64_t _id)
{
  this->pluginName = _pluginName;
  this->pluginUri = _pluginUri;
  this->pluginDescription = _pluginDescription;
  this->pluginCopyright = _pluginCopyright;
  this->creatorName = _creatorName;
  this->creatorURL = _creatorURL;
  this->id = _id;
}
std::string_view
APAL::StaticInfoComponent::getPluginName()
{
  return this->pluginName;
}
std::string_view
APAL::StaticInfoComponent::getPluginURI()
{
  return this->pluginUri;
}
std::string_view
APAL::StaticInfoComponent::getPluginDescription()
{
  return this->pluginDescription;
}
std::string_view
APAL::StaticInfoComponent::getPluginCopyright()
{
  return this->pluginCopyright;
}
std::string_view
APAL::StaticInfoComponent::getCreatorName()
{
  return this->creatorName;
}
std::string_view
APAL::StaticInfoComponent::getCreatorURL()
{
  return this->creatorURL;
}

int64_t
APAL::StaticInfoComponent::getID()
{
  return this->id;
}
