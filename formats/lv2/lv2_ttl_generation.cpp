#include "lv2_ttl_generation.hpp"
#include <sstream>
//#include <iostream>
#include <fstream>  
#include <cstring>
#include <tools/PortHandling.hpp>
#include <tools/StringTools.hpp>
#include <interfaces/Ports/IAudioPort.hpp>
using namespace XPlug;

std::string mapSpeakerConfToLV2Type(SpeakerConfiguration conf) {
    switch (conf) {
    case SpeakerConfiguration::Mono:
        return "MonoGroup";
    }
    return "";
}
std::string getTTLFromPlugin(IPlugin* pluginPtr)
{
    auto plug = static_cast<IPlugin*>(pluginPtr);
    std::stringstream plugTTL;

    plugTTL << "@prefix doap : <http://usefulinc.com/ns/doap#> ." << std::endl
        << "@prefix lv2 : <http://lv2plug.in/ns/lv2core#> ." << std::endl
        << "@prefix rdf : <http://www.w3.org/1999/02/22-rdf-syntax-ns#> ." << std::endl
        << "@prefix rdfs : <http://www.w3.org/2000/01/rdf-schema#> ." << std::endl
        << "@prefix units : <http://lv2plug.in/ns/extensions/units#> ." << std::endl
        << "@prefix pg : <http://lv2plug.in/ns/ext/port-groups#>." << std::endl
        << ((plug->getFeatureComponent()->supportsFeature(Feature::MidiInput) || plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput)) ? "@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .\n" : "")
        << "<" << plug->getInfoComponent()->getPluginURI() << ">" << std::endl
        << "    a lv2:Plugin ; " << std::endl
        //TODO: implement speacial style of Plugin here. (like lv2 : AmplifierPlugin;)
        << "    lv2:project <" << plug->getInfoComponent()->getCreatorURL() << "> ;" << std::endl
        << "    doap:name \"" << plug->getInfoComponent()->getPluginName() << "\" ;" << std::endl
        << "    doap:license <" << plug->getInfoComponent()->getPluginCopyright() << "> ;" << std::endl;

    /******DETECT AND WRITE PORTS*****/
    //TODO maybe add  "lv2:optionalFeature lv2:hardRTCapable ;" here
    auto portsSize = getNumberOfPorts<IAudioPort>(plug, PortDirection::Input) + getNumberOfPorts<IAudioPort>(plug, PortDirection::Output);
    iteratePorts<IPort>(plug, [&plugTTL, portsSize, plug](IPort* p, size_t index) {
        if (dynamic_cast<IAudioPort*>(p) != nullptr) {
            auto aPort = dynamic_cast<IAudioPort*>(p);
            for (int i = 0; i < aPort->size(); i++) {
                std::string symbol = to_string(aPort->at(i)->getName());
                std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
                std::string name = symbol;
                name[0] = std::toupper(name[0]);
                plugTTL << "    lv2:port [" << std::endl 
                    << "        a lv2:AudioPort ," << std::endl
                    << "            " << (p->getDirection() == PortDirection::Input ? "lv2:InputPort ;" : "lv2:OutputPort ;") << std::endl
                    << "        " << "lv2:symbol " << symbol << ";" << std::endl
                    << "        " << "lv2:name " << name << ";" << std::endl;
            }
        }
        else if (dynamic_cast<IMidiPort*>(p) != nullptr) {
            if ((p->getDirection() == PortDirection::Input && plug->getFeatureComponent()->supportsFeature(Feature::MidiInput)) ||
                (p->getDirection() == PortDirection::Output && plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput))) {
                std::string symbol = to_string(p->getPortName());
                std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
                std::string name = symbol;
                name[0] = std::toupper(name[0]);
                plugTTL << "    lv2:port [" << std::endl
                    << "        a atom:AtomPort  ," << std::endl
                    << "            " << (p->getDirection() == PortDirection::Input ? "lv2:InputPort ;" : "lv2:OutputPort ;") << std::endl
                    << "        atom:bufferType atom:Sequence ;" << std::endl
                    << "        atom:supports midi:MidiEvent ;" << std::endl
                    << "        " << "lv2:symbol " << symbol << ";" << std::endl
                    << "        " << "lv2:name " << name << ";" << std::endl;
            }
        }
        plugTTL << "        " << "lv2:index " << std::to_string(index) << ";" << std::endl
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
        mainTTLs << "<" << plug.plugPtr->getInfoComponent()->getPluginURI() << ">" << std::endl
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
        plugins[i].ttlFileName = std::string(GlobalData().getPlugin(i)->getInfoComponent()->getPluginName()) + ".ttl";
    }
    return plugins;
}

