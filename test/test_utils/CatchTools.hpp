#ifndef UNIT_TOOLS_HPP
#define UNIT_TOOLS_HPP
#include "catch.hpp"
#define CHECK_MESSAGE(cond, msg) { INFO(msg); CHECK(cond); }
#define REQUIRE_MESSAGE(cond, msg)  { INFO(msg); REQUIRE(cond); }
#define ASSERT_MESSAGE(msg) { INFO(msg); REQUIRE(false); }
#endif //! UNIT_TOOLS_HPP