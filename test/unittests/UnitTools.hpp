#ifndef UNIT_TOOLS_HPP
#define UNIT_TOOLS_HPP
#define CHECK_MESSAGE(cond, msg) { INFO(msg); CHECK(cond); }
#define REQUIRE_MESSAGE(cond, msg)  { INFO(msg); REQUIRE(cond); }
#endif //! UNIT_TOOLS_HPP