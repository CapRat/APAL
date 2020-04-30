#include "../formats/ladspa/ladspa.h"
#include "catch2/catch.hpp"
#include "lib_loading.hpp"

#define LADSPA_TEST_LIB "ExportLib"
library_t ladspaLib = LoadLib(LADSPA_TEST_LIB);
LADSPA_Descriptor_Function pfDescriptorFunction = LoadFunc<LADSPA_Descriptor_Function>(ladspaLib, "ladspa_descriptor");

unsigned long lPortIndex;
unsigned long lSpaceIndex;
unsigned long lSpacePadding1;
unsigned long lSpacePadding2;
unsigned long lLength;
void* pvPluginHandle;
LADSPA_PortRangeHintDescriptor iHintDescriptor;
LADSPA_Data fBound;
LADSPA_Data fDefault;

TEST_CASE("SIMPLE LADSPA LOADING")
{
    REQUIRE(pfDescriptorFunction != nullptr);
    const LADSPA_Descriptor* psDescriptor = nullptr;
    unsigned long lPluginIndex;
    for (lPluginIndex = 0;; lPluginIndex++) {
        psDescriptor = pfDescriptorFunction(lPluginIndex);
        if (!psDescriptor)
            break;
        REQUIRE(psDescriptor->Name != nullptr);
    }
    REQUIRE(psDescriptor != nullptr);
}
