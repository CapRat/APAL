/**
 * Used for building an shared library.
 */
#include "simple_symbol_export_export.h"
#include "static_example.hpp"

extern "C" {

int SIMPLE_SYMBOL_EXPORT_EXPORT test_function()
{
    return 2;
}
}
