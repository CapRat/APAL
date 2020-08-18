/**
 * @file groups all IFeatureComponent implementations.
 */

#ifndef FEATURE_COMPONENTS_HPP
#define FEATURE_COMPONENTS_HPP
#include <algorithm>
#include <array>
#include <interfaces/IFeatureComponent.hpp>
#include <string>
#include <vector>
namespace APAL {
class IPortComponent;
/**
 * @brief Exception, which is called, when a feature is not supported
 */
struct UseOfNonSupportedFeature : public std::exception
{
  UseOfNonSupportedFeature(Feature f)
    : feat(f)
  {}
  const char* what() const noexcept
  {
    return "invalid use of Feature with value:";
  }

  Feature feat;
};

/**
 * @brief Static Feature Component, wiht is initialized with a fixed number of
 * Features. Its implemented with an std::array.
 */
template<size_t numOfFeatures>
class StaticFeatureComponent : public IFeatureComponent
{
public:
  StaticFeatureComponent(std::array<Feature, numOfFeatures> features)
    : supportedFeatures(features)
  {}

  // Geerbt über IFeatureComponent
  inline virtual bool supportsFeature(Feature feature) override
  {
    return std::find(supportedFeatures.begin(),
                     supportedFeatures.end(),
                     feature) != supportedFeatures.end();
  }

  inline virtual void formatNotSupportedFeature(Feature feature,
                                                std::string format) override
  {
    throw UseOfNonSupportedFeature(feature);
  }

protected:
  std::array<Feature, numOfFeatures> supportedFeatures;
};

/**
 * @brief Implementation of the IFeatureComponent with an dynamic
 * array(std::vector), so features can be dynamicly added.
 */
class DynamicFeatureComponent : public IFeatureComponent
{
public:
  DynamicFeatureComponent() = default;
  DynamicFeatureComponent(std::vector<Feature> features);

  void addSupportedFeature(Feature feat);

  // Geerbt über IFeatureComponent
  virtual bool supportsFeature(Feature feature) override;
  virtual void formatNotSupportedFeature(Feature feature,
                                         std::string format) override;

protected:
  std::vector<Feature> supportedFeatures;
};

/**
 * @brief Extends the Function of the DynamicFeatureComponent with the automatic
 * Detection of Features. For this to work, Information about other Components
 * is required, so they should be inserted in the Constructor. Also Features can
 * additionally added, because the AutomaticFeatureComponent is a
 * DynamicFeatureComponent.
 */
class AutomaticFeatureComponent : public DynamicFeatureComponent
{
public:
  /**
   * @brief Constructor, which calls automaticly detectFeaturs. If something
   * changed on the components, call  detectFeatures afterwards.
   * @param pComp PortComponent, where features like about Midi and so on are
   * extracted from.
   * @param features additional features.
   */
  AutomaticFeatureComponent(IPortComponent* pComp,
                            std::vector<Feature> features = {});
  void detectFeatures(IPortComponent* pComp);
};
}
#endif // !FEATURE_COMPONENTS_HPP
