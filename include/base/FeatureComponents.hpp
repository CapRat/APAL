#ifndef FEATURE_COMPONENTS_HPP
#define FEATURE_COMPONENTS_HPP
#include <interfaces/IFeatureComponent.hpp>
#include <array>
#include <algorithm>
#include <string>
#include <vector>
namespace XPlug {
    class IPortComponent;
    struct UseOfNonSupportedFeature : public std::exception {
        UseOfNonSupportedFeature(Feature f) :feat(f) {}
        const char* what() const noexcept
        {
            return "invalid use of Feature with value:";
        }

        Feature feat;
    };
    
    template <size_t numOfFeatures>
    class StaticFeatureComponent :public IFeatureComponent {
    public:
        StaticFeatureComponent(std::array<Feature, numOfFeatures> features) :supportedFeatures(features) {
        }

        // Geerbt über IFeatureComponent
        inline  virtual bool supportsFeature(Feature feature) override
        {
            return std::find(supportedFeatures.begin(), supportedFeatures.end(), feature) != supportedFeatures.end();
        }

        inline virtual void formatNotSupportedFeature(Feature feature, std::string format) override
        {
            throw UseOfNonSupportedFeature(feature);
        }

    protected:
        std::array<Feature, numOfFeatures> supportedFeatures;

    };

    class DynamicFeatureComponent :public IFeatureComponent {
    public:
        DynamicFeatureComponent() = default;
        DynamicFeatureComponent(std::vector<Feature> features);

        void addSupportedFeature(Feature feat);

        // Geerbt über IFeatureComponent
        virtual bool supportsFeature(Feature feature) override;
        virtual void formatNotSupportedFeature(Feature feature, std::string format) override;

    protected:
        std::vector<Feature>  supportedFeatures;

    };
    class AutomaticFeatureComponent :public DynamicFeatureComponent {
    public:
        AutomaticFeatureComponent();
     //   AutomaticFeatureComponent(IPortComponent* pComp);
        AutomaticFeatureComponent(IPortComponent* pComp, std::vector<Feature> features = { });
        void detectFeatures(IPortComponent* pComp);
    };
}
#endif // !FEATURE_COMPONENTS_HPP
