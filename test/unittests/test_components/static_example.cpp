/**Static testlibrary, to link this static function in an shared library and check if function is inside of it**/

#include "static_example.hpp"

int static_test_function(int x)
{
    return 5 + x;
}

extern "C" {
int static_test_function2(int x)
{
    return 5 + x;
}
}
