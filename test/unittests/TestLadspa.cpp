#include "../formats/ladspa/ladspa.h"
#include "TestLoading.hpp"
#include "catch2/catch.hpp"
#define LADSPA_TEST_LIB "ExportLib"

//library_t ladspaLib;
//LADSPA_Descriptor_Function pfDescriptorFunction;
unsigned long lPortIndex;
unsigned long lSpaceIndex;
unsigned long lSpacePadding1;
unsigned long lSpacePadding2;
unsigned long lLength;
void* pvPluginHandle;
LADSPA_PortRangeHintDescriptor iHintDescriptor;
LADSPA_Data fBound;
LADSPA_Data fDefault;

/*TEST_CASE("SIMPLE LADSPA LOADING")
{
    library_t ladspaLib = LoadTestLib(LADSPA_TEST_LIB);
    LADSPA_Descriptor_Function pfDescriptorFunction = LoadFunc<LADSPA_Descriptor_Function>(ladspaLib, "ladspa_descriptor");
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
}*/
