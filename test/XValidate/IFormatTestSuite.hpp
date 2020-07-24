#ifndef I_FORMAT_TEST_HPP
#define I_FORMAT_TEST_HPP
#include <string>
#include <memory>
#include "XValidate.hpp"
enum class VerbosityLevel {
    Verbose,
    Normal,
    Quiet
};
struct TestSuiteData {
    std::string pluginPath;
    VerbosityLevel verbosityLevel;
    int strictnessLevel;
};
typedef int SucceedState ;
#define TEST_SUCCEEDED 0
#define TEST_FAILED 1
#define OPT_TEST_FAILED 2
class IFormatTestSuite {
public:
    virtual ~IFormatTestSuite() = default;
    /**
     * @brief Gets the Name of the Format, which the suite tests.
     * @return the Name of the Format, which the suite tests. Like LADSPA or LV2
    */
    virtual std::string getFormatName() = 0;

    /**
     * @brief Initializes the Testsuite with needed data. Dont do something fancy in here. Every testsuite is initialized, because it should be tested through isSupported.
     * @param data Data for initializing.
     */
    virtual void initialize(TestSuiteData data) = 0;

    /**
     * @brief Is needed, when the automatic detection of formats is enabled. This Function indicates, 
     * weather the given testobjects, meet the requirements to be tested or not. This Function is called before initialize is called.
     * @return 
    */
    virtual bool isSupported(std::string pluginPath) = 0;

    /**
     * @brief Enable the Suite to run tests.
     */
    virtual void enable() = 0;
    /**
     * @brief Disable the Suite to run tests.
     */
    virtual void disable() = 0;

    /**
     * @brief returns true, if the TestSuite is enabled, false if the Suite is disabled.
     * @return returns true, if the TestSuite is enabled, false if the Suite is disabled.
     */
    virtual bool isEnabled() = 0;

    /**
     * @brief Runs the Testsuite, with a given Strictnesslevel. 
     * @param strictnessLevel The Higher the StrictnessLevel, the harder the tests are to pass.
     * @return Returncode. 0 Is succeeded, everything else is an error.
     */
    virtual SucceedState run() = 0;

    /**
     * @brief Runs the Performancetests of the Suite
     * @return Returncode. 0 Is succeeded, everything else is an error.
     */
    virtual SucceedState runPerformance() = 0;
};


template <typename TestType>
class TestSuiteRegistrator {
public:
    std::shared_ptr<IFormatTestSuite> testSuitePtr = nullptr;
    TestSuiteRegistrator()
    {
        testSuitePtr = std::shared_ptr<IFormatTestSuite>(new TestType());
        RegisterTestSuite(testSuitePtr);
    }
};

#define REGISTER_TEST_SUITE(TestClassName) static TestSuiteRegistrator<TestClassName> \
                    instance##TestClassName = TestSuiteRegistrator<TestClassName>()


class FormatTestSuiteBase :public IFormatTestSuite {
protected:
    bool enabled;
    TestSuiteData data;
public:
    virtual ~FormatTestSuiteBase() = default;

    inline  virtual void initialize(TestSuiteData data) override { this->data = data; }
    inline virtual void enable() override { this->enabled = true; }
    inline virtual void disable() override { this->enabled = false; }
    inline virtual bool isEnabled() override { return   enabled; }
    inline virtual SucceedState test(int strictnessLevel, bool cond, std::string failMsg) {
        if (strictnessLevel <= this->data.strictnessLevel) {
            if (!cond) {
                GlobalLog().logN(failMsg, LoggerValue::ERROR);
                return TEST_FAILED;
            }
        }
        return TEST_SUCCEEDED;

    }
};


#endif //! I_FORMAT_TEST_HPP