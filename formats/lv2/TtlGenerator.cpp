/**
 * @file TtlGenerator helper application, to genereate LV2 ttl files.
 */
#include "lv2_ttl_generation.hpp"
#include <fstream>
#include <interfaces/IPlugin.hpp>
#include <iostream>
#include <sstream>
#include <tools/LibLoading.hpp>
#include <tools/StringTools.hpp>

using namespace APAL;
/********FUNCTION VARIABLES*********/
std::string (*getTTLFromPluginPtr)(IPlugin*) = nullptr;
std::string (*getManifestFromMultpleInfosPtr)(std::vector<TTLPluginInfo>) =
  nullptr;
std::vector<TTLPluginInfo> (*getPluginInfosPtr)(const std::string&) = nullptr;
void
writeOutBundle(std::vector<TTLPluginInfo> infoArray,
               const std::string& fileOutDir = "./")
{
  for (auto info : infoArray) {
    auto plugTTL = getTTLFromPluginPtr(info.plugPtr);
    std::ofstream outfile(fileOutDir + info.ttlFileName);
    outfile << plugTTL;
    outfile.close();
  }
  auto manifestTTL = getManifestFromMultpleInfosPtr(infoArray);
  std::ofstream outfile(fileOutDir + "manifest.ttl");
  outfile << manifestTTL;
  outfile.close();
}

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Use: TTLGenerator bundlePath LV2LibPath";
    return 0;
  }

  /*********LOAD LIBRARY AND FUNCTIONS********/
  std::string pluginLib(argv[1]);
  APAL::replaceInString(pluginLib, "\\", "/");
  auto pluginLibHandle = LoadLib(pluginLib.c_str());
  if (pluginLibHandle == nullptr) {
    std::cerr << GetErrorStr() << std::endl;
    return 1;
  }

  getTTLFromPluginPtr =
    LoadFunc<std::string (*)(IPlugin*)>(pluginLibHandle, "getTTLFromPlugin");
  getManifestFromMultpleInfosPtr =
    LoadFunc<std::string (*)(std::vector<TTLPluginInfo>)>(
      pluginLibHandle, "getManifestFromMultpleInfos");
  getPluginInfosPtr =
    LoadFunc<std::vector<TTLPluginInfo> (*)(const std::string&)>(
      pluginLibHandle, "getPluginInfos");

  /***************GET INFORMATIONS*****************/
  auto y = GetErrorStr();
  std::cout << y;
  auto infos = getPluginInfosPtr(getFileName(pluginLib, true));

  writeOutBundle(infos);
}