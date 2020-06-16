#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include "XValidate.hpp"
using namespace Catch;
std::string pluginPath;
bool testVST2=false, testVST3 = false, testLADSPA = false, testLV2 = false;
clara::Parser makeMyCMDParser(ConfigData& config) {

    using namespace clara;

  
    auto const setColourUsage = [&](std::string const& useColour) {
        auto mode = toLower(useColour);

        if (mode == "yes")
            config.useColour = UseColour::Yes;
        else if (mode == "no")
            config.useColour = UseColour::No;
        else if (mode == "auto")
            config.useColour = UseColour::Auto;
        else
            return ParserResult::runtimeError("colour mode must be one of: auto, yes or no. '" + useColour + "' not recognised");
        return ParserResult::ok(ParseResultType::Matched);
    };

    auto const setVerbosity = [&](std::string const& verbosity) {
        auto lcVerbosity = toLower(verbosity);
        if (lcVerbosity == "quiet")
            config.verbosity = Verbosity::Quiet;
        else if (lcVerbosity == "normal")
            config.verbosity = Verbosity::Normal;
        else if (lcVerbosity == "high")
            config.verbosity = Verbosity::High;
        else
            return ParserResult::runtimeError("Unrecognised verbosity, '" + verbosity + "'");
        return ParserResult::ok(ParseResultType::Matched);
    };
    auto const setReporter = [&](std::string const& reporter) {
        IReporterRegistry::FactoryMap const& factories = getRegistryHub().getReporterRegistry().getFactories();

        auto lcReporter = toLower(reporter);
        auto result = factories.find(lcReporter);

        if (factories.end() != result)
            config.reporterName = lcReporter;
        else
            return ParserResult::runtimeError("Unrecognized reporter, '" + reporter + "'. Check available with --list-reporters");
        return ParserResult::ok(ParseResultType::Matched);
    };
  
    auto cli
        = ExeName(config.processName)
        | Help(config.showHelp)
    
        | Opt(config.outputFilename, "filename")
        ["-o"]["--out"]
        ("output filename")
        | Opt(setReporter, "name")
        ["-r"]["--reporter"]
        ("reporter to use (defaults to console)")
        | Opt(config.name, "name")
        ["-n"]["--name"]
        ("name of the Testsuit (name in junit files)")
        | Opt(setVerbosity, "quiet|normal|high")
        ["-v"]["--verbosity"]
        ("set output verbosity")
        | Opt(config.listReporters)
        ["--list-reporters"]
        ("list all reporters")
        | Opt(setColourUsage, "yes|no")
        ["--use-colour"]
        ("should output be colourised")
        | Opt(testLADSPA)
        ["-l1"]["--ladspa"]
        ("Run LADSPA validation tests")
        | Opt(testLADSPA)
        ["-l2"]["--LV2"]
        ("Run LV2 validation tests")
        | Opt(testVST2)
        ["-v2"]["--VST2"]
        ("Run VST2 validation tests")
        | Opt(testVST3)
        ["-v3"]["--VST3"]
        ("Run VST3 validation tests")
        | Arg(pluginPath, "plugin_path")
        ("Full Path to the Plugin, so it can be loaded");

    return cli;
}

int main(int argc, char* argv[])
{
    Catch::Session session; // There must be exactly one instance
    // Build a new parser on top of Catch's
    using namespace Catch::clara;
  //  auto cli = session.cli();
    auto cli = makeMyCMDParser(session.configData());
    
    
  // Now pass the new composite back to Catch so it uses that
    session.cli(cli);


   // session.configData().sectionsToRun=""; //specify section to run



    int returnCode = session.applyCommandLine(argc, argv); //Lets Catch takeover
    if (returnCode != 0)
        return returnCode;

    session.configData().runOrder = RunTests::InDeclarationOrder;
    session.configData().waitForKeypress = WaitForKeypress::Never;
    session.configData().listTests = false; //list all tests.
    session.configData().listTags = false; // list all/matching tags
    session.configData().showSuccessfulTests = false; //include successful tests in output
    session.configData().shouldDebugBreak = false; // break into debugger on failure
    session.configData().noThrow = false; // skip exception tests
    session.configData().showInvisibles = false; // show invisibles (tabs, newlines)
    session.configData().abortAfter = 1; // Abort after X failures.
    session.configData().libIdentify = false; //report name and version according to libidentify standard
    session.configData().benchmarkSamples = 100; //number of samples to collect (default: 100)
    session.configData().benchmarkResamples = 100000;//number of resamples for the bootstrap (default: 100000)
    session.configData().benchmarkConfidenceInterval = 0.95; //confidence interval for the bootstrap (between 0 and 1, default: 0.95)
    session.configData().benchmarkNoAnalysis = false;//perform only measurements; do not perform any analysis
    session.configData().benchmarkWarmupTime = 100;//amount of time in milliseconds spent on warming up each test (default: 100)
    session.configData().listTestNamesOnly = false; //list all/matching test cases names only
    session.configData().showDurations = ShowDurations::DefaultForReporter;//show test durations
    session.configData().warnings = static_cast<WarnAbout::What>(WarnAbout::NoTests | WarnAbout::NoAssertions);//Enables reporting of suspicious test states. 
    //NoAssertions  -->  // Fail test case / leaf section if no assertions  (e.g. `REQUIRE`) is encountered.
    //NoTests       --> Return non-zero exit code when no test cases were run. Also calls reporter's noMatchingTestCases method
    session.configData().filenamesAsTags = false; //adds a tag for the filename
    if (testVST2)
        session.configData().testsOrTags.push_back("[vst2],");
    if (testVST3)
        session.configData().testsOrTags.push_back("[vst3],");
    if (testLADSPA)
        session.configData().testsOrTags.push_back("[ladspa],");
    if (testLV2)
        session.configData().testsOrTags.push_back("[lv2],");

    return session.run();
}