#include <interfaces/IPlugin.hpp>
#include <sstream>
#include <iostream>
#include <fstream>  
#include <tools/LibLoading.hpp>
#include <tools/StringTools.hpp>
#include "lv2_ttl_generation.hpp"

using namespace XPlug;

std::string(*getTTLFromPluginPtr)(IPlugin*) = nullptr;
std::string(*getManifestFromMultpleInfosPtr)(std::vector<TTLPluginInfo>) = nullptr;
std::vector<TTLPluginInfo>(*getPluginInfosPtr)(std::string) = nullptr;



void writeOutBundle(std::vector<TTLPluginInfo> infoArray,std::string fileOutDir ="./") {
    for (auto info : infoArray) {
        auto plugTTL= getTTLFromPluginPtr(info.plugPtr);
        std::ofstream outfile(fileOutDir + info.ttlFileName);
        outfile << plugTTL;
        outfile.close();
    }
    auto manifestTTL = getManifestFromMultpleInfosPtr(infoArray);
    std::ofstream outfile(fileOutDir + "manifest.ttl");
    outfile << manifestTTL;
    outfile.close();
}

int main(int argc, char* argv[]) { 
    if (argc < 2) {
        std::cout << "Use: TTLGenerator bundlePath LV2LibPath";
        return 0;
    }
    
    /*********LOAD LIBRARY AND FUNCTIONS********/
    std::string pluginLib(argv[1]);
    auto pluginLibHandle = LoadLib(pluginLib.c_str());
    if (pluginLibHandle == nullptr) {
        std::cerr << GetErrorStr() << std::endl;
        return 1;
    }
 
    getTTLFromPluginPtr = LoadFunc<std::string(*)(IPlugin*)>(pluginLibHandle, "getTTLFromPlugin");
    getManifestFromMultpleInfosPtr = LoadFunc< std::string(*)(std::vector<TTLPluginInfo>)>(pluginLibHandle, "getManifestFromMultpleInfos");
    getPluginInfosPtr = LoadFunc< std::vector<TTLPluginInfo>(*)(std::string)>(pluginLibHandle, "getPluginInfos");


    /***************GET INFORMATIONS*****************/

    auto infos = getPluginInfosPtr(getFileName(pluginLib, true));

    writeOutBundle(infos);
   
}