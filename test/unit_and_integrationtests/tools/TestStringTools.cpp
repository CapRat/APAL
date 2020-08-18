#include "CatchTools.hpp"
#include <tools/StringTools.hpp>
using namespace APAL;
TEST_CASE("getFileName Test")
{
  std::string extPath = "C://test/file/path.ext";
  std::string winPath = "C:\\test\\file\\path.ext";
  REQUIRE_MESSAGE(getFileName(extPath) == "path.ext", "Error, wrong filename");
  REQUIRE_MESSAGE(getFileName(extPath, false) == "path",
                  "Error, wrong filename");
  REQUIRE_MESSAGE(getFileName(extPath, false, '\\') == extPath,
                  "Error, wrong filename");
  REQUIRE_MESSAGE(getFileName(winPath, true, '\\') == "path.ext",
                  "Error, wrong filename");
  REQUIRE_MESSAGE(getFileName(winPath, false, '\\') == "path",
                  "Error, wrong filename");
}

TEST_CASE("ReplaceinString Test")
{
  std::string extPath = "/test/file/path.ext";
  REQUIRE_MESSAGE(replaceInString(extPath, "/", "\\") ==
                    "\\test\\file\\path.ext",
                  "Error, couldnt replace stuff");
  REQUIRE_MESSAGE(extPath == "/test/file/path.ext",
                  "Error, original String was modified");
}