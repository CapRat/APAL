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
XPlug::StaticInfoComponent::StaticInfoComponent(std::string _pluginName, std::string _pluginUri, std::string _pluginDescription, std::string _pluginCopyright, std::string _creatorName, std::string _creatorURL)
{
    this->pluginName = _pluginName;
    this->pluginUri = _pluginUri;
    this->pluginDescription = _pluginDescription;
    this->pluginCopyright = _pluginCopyright;
    this->creatorName = _creatorName;
    this->creatorURL = _creatorURL;
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