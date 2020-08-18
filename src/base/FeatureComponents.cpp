#include "base/FeatureComponents.hpp"
#include "tools/PortHandling.hpp"
using namespace APAL;
APAL::DynamicFeatureComponent::DynamicFeatureComponent(
  std::vector<Feature> features)
  : supportedFeatures(std::move(features))
{}
bool
APAL::DynamicFeatureComponent::supportsFeature(Feature feature)
{
  return std::find(supportedFeatures.begin(),
                   supportedFeatures.end(),
                   feature) != supportedFeatures.end();
}
void
APAL::DynamicFeatureComponent::formatNotSupportedFeature(Feature feature,
                                                          std::string)
{
  throw UseOfNonSupportedFeature(feature);
}
void
APAL::DynamicFeatureComponent::addSupportedFeature(Feature feat)
{
  this->supportedFeatures.push_back(feat);
}


APAL::AutomaticFeatureComponent::AutomaticFeatureComponent(
  IPortComponent* pComp,
  std::vector<Feature> features)
  : DynamicFeatureComponent::DynamicFeatureComponent(features)
{
  this->detectFeatures(pComp);
}

void
APAL::AutomaticFeatureComponent::detectFeatures(IPortComponent* pComp)
{
  // bool suppAudioPort = false;
  bool suppMidiIn = false;
  bool suppMidiOut = false;
  for (size_t i = 0; i < pComp->size(); i++) {
    auto port = pComp->at(i);
    if (dynamic_cast<IAudioPort*>(port)) {
      //   suppAudioPort = true;
    } else if (dynamic_cast<IMidiPort*>(port)) {
      if (port->getDirection() == PortDirection::Input)
        suppMidiIn = true;
      else
        suppMidiOut = true;
    }
  }
  if (suppMidiIn)
    this->addSupportedFeature(Feature::MidiInput);
  if (suppMidiOut)
    this->addSupportedFeature(Feature::MidiOutput);
}