#include "lv2_ttl_generation.hpp"
#include <sstream>
//#include <iostream>
#include <fstream>  
#include <cstring>
#include <tools/PortHandling.hpp>
#include <tools/StringTools.hpp>
#include <interfaces/IAudioPort.hpp>

using namespace XPlug;


std::string getTTLFromPlugin(IPlugin* pluginPtr)
{
    auto plug = static_cast<IPlugin*>(pluginPtr);
    std::stringstream plugTTL;
    std::string devHP = plug->getPluginInfo()->url;
    plugTTL << "@prefix doap : < http ://usefulinc.com/ns/doap#> ." << std::endl
        << "@prefix lv2 : < http ://lv2plug.in/ns/lv2core#> ." << std::endl
        << "@prefix rdf : < http ://www.w3.org/1999/02/22-rdf-syntax-ns#> ." << std::endl
        << "@prefix rdfs : < http ://www.w3.org/2000/01/rdf-schema#> ." << std::endl
        << "@prefix units : < http ://lv2plug.in/ns/extensions/units#> ." << std::endl
        << ((plug->getFeatureComponent()->supportsFeature(Feature::MidiInput) || plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput)) ? "@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .\n" : "")
        << "<" << plug->getPluginInfo()->url << ">" << std::endl
        << "    a lv2:Plugin ; " << std::endl
        //TODO: implement speacial style of Plugin here. (like lv2 : AmplifierPlugin;)
        << "    lv2:project <" << plug->getPluginInfo()->creater.url << "> ;" << std::endl
        << "    doap:name \"" << plug->getPluginInfo()->name << "\" ;" << std::endl
        << "    doap:license <" << plug->getPluginInfo()->copyright << "> ;" << std::endl;
    //TODO maybe add  "lv2:optionalFeature lv2:hardRTCapable ;" here
    auto portsSize = getNumberOfPorts<IAudioPort>(plug, PortDirection::Input) + getNumberOfPorts<IAudioPort>(plug, PortDirection::Output);
    iteratePortsFiltered<IPort>(plug, [&plugTTL, portsSize, plug](IPort* p, size_t index) {
        std::string symbol = to_string(p->getPortName());
        std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
        std::string name = symbol;
        name[0] = std::toupper(name[0]);
        plugTTL << "    lv2:port [" << std::endl;
        if (dynamic_cast<IAudioPort*>(p) != nullptr) {
            plugTTL << "        a lv2:AudioPort ," << std::endl
                << "            " << (p->getDirection() == PortDirection::Input ? "lv2:InputPort ;" : "lv2:OutputPort ;") << std::endl;
        }
        else if (dynamic_cast<IMidiPort*>(p) != nullptr) {
            if ((p->getDirection() == PortDirection::Input && plug->getFeatureComponent()->supportsFeature(Feature::MidiInput)) ||
                (p->getDirection() == PortDirection::Output && plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput))) {
                plugTTL << "        a atom:AtomPort  ," << std::endl
                    << "            " << (p->getDirection() == PortDirection::Input ? "lv2:InputPort ;" : "lv2:OutputPort ;") << std::endl
                    << "        atom:bufferType atom:Sequence ;" << std::endl
                    << "        atom:supports midi:MidiEvent ;" << std::endl;
            }
        }
        plugTTL << "        " << "lv2:index " << std::to_string(index) << ";" << std::endl
            << "        " << "lv2:symbol " << symbol << ";" << std::endl
            << "        " << "lv2:name " << name << ";" << std::endl
            << "    ]" << (index == portsSize - 1 ? "." : ",") << std::endl
            ;
        return false;
        });
    //std::strcpy(ttlFile,plugTTL.str().c_str());
    return  plugTTL.str();
}

std::string getManifestFromMultpleInfos(std::vector<TTLPluginInfo> plugInfos)
{
    std::stringstream mainTTLs;
    mainTTLs << "@prefix lv2 : < http ://lv2plug.in/ns/lv2core#> ." << std::endl
        << "@prefix rdfs : < http ://www.w3.org/2000/01/rdf-schema#> ." << std::endl;

    for (auto plug:plugInfos){
        mainTTLs << "<" << plug.plugPtr->getPluginInfo()->url << ">" << std::endl
            << "    a lv2:Plugin ;" << std::endl
            << "    lv2:binary <" << plug.binFileName << "> ;" << std::endl
            << "    rdfs:seeAlso <" << plug.ttlFileName << "> ." << std::endl;
    }
    return mainTTLs.str();
}

size_t getSizeOfPluginInfos()
{
    return GlobalData().getNumberOfRegisteredPlugins();
}

std::vector<TTLPluginInfo> getPluginInfos(std::string binaryPath)
{
    std::vector<TTLPluginInfo> plugins(GlobalData().getNumberOfRegisteredPlugins());
    for (size_t i = 0; i < plugins.size(); i++) {
        plugins[i].plugPtr = GlobalData().getPlugin(i).get();
        plugins[i].binFileName = binaryPath;
        plugins[i].ttlFileName = GlobalData().getPlugin(i)->getPluginInfo()->name + ".ttl";
    }
    return plugins;
}

