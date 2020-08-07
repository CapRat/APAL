#include "clipp.hpp"
#include "tools/Logger.hpp"
#include "XValidate.hpp"
#include "IFormatTestSuite.hpp"
#include <vector>
#include <algorithm>
struct GlobalData_t {
    std::vector<std::shared_ptr<IFormatTestSuite>> testSuites;
};

GlobalData_t& GlobalData()
{
    static GlobalData_t* globalDataInstance = new GlobalData_t();
    return *globalDataInstance;
}


void RegisterTestSuite(std::shared_ptr<IFormatTestSuite> testSuite) {
    GlobalData().testSuites.push_back(testSuite);
}

IFormatTestSuite* GetTestSuite(const std::string& formatName)
{
    for (auto testSuite : GlobalData().testSuites)
        if (testSuite->getFormatName() == formatName)
            return testSuite.get();
    return nullptr;
}

Logger& GlobalLog()
{
    static Logger* globalLogger = new Logger();
    return *globalLogger;
}

using namespace clipp;
int main(int argc, char* argv[])
{
    for (auto testSuite : GlobalData().testSuites) {
        testSuite->disable();
    }
    std::string pluginPath;
    bool enablePerformanceMeasureing=false;
    bool disableTesting=false;
    int strictnessLevel = 0;
    bool verboseLogging=false;
    bool silentLogging = false;
    bool testDefaultSelection = true;
    auto cli = (
        option("-p", "--performance").set(enablePerformanceMeasureing).doc("Enables performancemeasurement of functions."),
        option("-w", "--verbose").set(verboseLogging).doc("Prints all available messages."),
        option("-q", "--quiet").set(silentLogging).doc("Prints nothing, except errors."),
        option("-nt","--notest").set(disableTesting).doc("Dont run regular tests."),
        option("-l", "--strictnessLevel").doc("Sets the Level, how strict the tests should be executed.") & value("strictness level", strictnessLevel) 
        );
    for (auto testSuite : GlobalData().testSuites) {
        auto formatName = testSuite->getFormatName();
        std::transform(formatName.begin(), formatName.end(), formatName.begin(), ::tolower);
        cli.push_back(option("-" + formatName, "--enable" + testSuite->getFormatName()).call([testSuite, &testDefaultSelection](const char*) {
            testSuite->enable(); testDefaultSelection = false;
            }) .doc("Enable " + testSuite->getFormatName() + " Tests."));
    }
    cli.push_back(value("path to pluginbinary", pluginPath));
    if (!parse(argc, argv, cli))  std::cout << make_man_page(cli, argv[0]);
    // If testDefaultSelection is true, enable all testsuites.
    TestSuiteData tData{ pluginPath , (verboseLogging?VerbosityLevel::Verbose:silentLogging?VerbosityLevel::Quiet:VerbosityLevel::Normal ),strictnessLevel };
   
    // Initialize all testsuites and enable or disable them, if default selection is needed.
    for (auto testSuite : GlobalData().testSuites) {
        if (testDefaultSelection) {
            if (testSuite->isSupported(pluginPath)) 
                testSuite->enable();
            else
                testSuite->disable();
        }
    }

    for (auto testSuite : GlobalData().testSuites) {
        if (testSuite->isEnabled()) {
            testSuite->initialize(tData);
            if (!disableTesting) {
                GlobalLog().logN("Execute " + testSuite->getFormatName() + " Tests with strictness " + std::to_string(strictnessLevel),LoggerValue::INFO);
                testSuite->run();
            }
     
            if (enablePerformanceMeasureing) {
                GlobalLog().logN("Execute Performanceanalysis for " + testSuite->getFormatName() + " Format",LoggerValue::INFO);
            }
        }
    }
}