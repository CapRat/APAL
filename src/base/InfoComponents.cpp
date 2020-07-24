#include <base/InfoComponents.hpp>
using namespace XPlug;
XPlug::StaticInfoComponent::StaticInfoComponent()
{
    this->pluginName = "";
    this->pluginUri = "";
    this->pluginDescription = "";
    this->pluginCopyright = "";
    this->creatorName = "";
    this->creatorURL = "";
}
XPlug::StaticInfoComponent::StaticInfoComponent(std::string pluginName, std::string pluginUri, std::string pluginDescription, std::string pluginCopyright, std::string creatorName, std::string creatorURL)
{
    this->pluginName = pluginName;
    this->pluginUri = pluginUri;
    this->pluginDescription = pluginDescription;
    this->pluginCopyright = pluginCopyright;
    this->creatorName = creatorName;
    this->creatorURL = creatorURL;
}
std::string_view XPlug::StaticInfoComponent::getPluginName()
{
    return this->pluginName;
}
std::string_view XPlug::StaticInfoComponent::getPluginURI()
{
    return this->pluginUri;
}
std::string_view XPlug::StaticInfoComponent::getPluginDescription()
{
    return this->pluginDescription;
}
std::string_view XPlug::StaticInfoComponent::getPluginCopyright()
{
    return this->pluginCopyright;
}
std::string_view XPlug::StaticInfoComponent::getCreatorName()
{
    return this->creatorName;
}
std::string_view XPlug::StaticInfoComponent::getCreatorURL()
{
    return this->creatorURL;
}