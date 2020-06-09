#ifndef XVALIDATE_HPP
#define XVALIDATE_HPP
#include <catch2/catch.hpp>
#include <string>
#define CHECK_MESSAGE(cond, msg) \
    {                            \
        INFO(msg);               \
        CHECK(cond);             \
    }
#define REQUIRE_MESSAGE(cond, msg) \
    {                              \
        INFO(msg);                 \
        REQUIRE(cond);             \
    }

#define REQUIRE_STRICT_MESSAGE(cond, msg) REQUIRE_MESSAGE(cond, msg)
#define STATUS_TASK(msg) Catch::cout() << "-- " << msg;
#define STATUS_RESULT(msg) Catch::cout() << " ... " << msg << std::endl;
#define STATUS_INFO(msg) Catch::cout() << msg << std::endl;

extern std::string pluginPath;
extern bool testVST2, testVST3, testLADSPA, testLV2;

#endif //!  XVALIDATE_HPP
