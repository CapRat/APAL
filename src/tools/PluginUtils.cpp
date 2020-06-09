#include <tools/PluginUtils.hpp>

size_t XPlug::getChannelInCount(IPlugin* plug) {
    size_t pCount = 0;
    for (Port& p : plug->getPortComponent()->getInputPorts()) {
        pCount += p.channels.size();
    }
    return pCount;
}

size_t XPlug::getChannelOutCount(IPlugin* plug) {
    size_t pCount = 0;
    for (Port& p : plug->getPortComponent()->getOutputPorts()) {
        pCount += p.channels.size();
    }
    return pCount;
}

size_t XPlug::getChannelCount(IPlugin* plug)
{
    return getChannelInCount(plug)+getChannelOutCount(plug);
}

void XPlug::iteratePorts(IPlugin* plug, std::function<bool(XPlug::Port&, size_t)> iterFunc)
{
    size_t counter = 0;
    for (Port& p : plug->getPortComponent()->getInputPorts()) {
        if (iterFunc(p, counter)) return;
        counter++;
    }
    for (Port& p : plug->getPortComponent()->getOutputPorts()) {
        if (iterFunc(p, counter)) return;
        counter++;
    }
}

void XPlug::iterateChannel(IPlugin* plug, std::function<bool(XPlug::Channel&, size_t)> iterFunc)
{
    size_t counter = 0;
    iteratePorts(plug, [&counter,&iterFunc](Port& p, size_t index) {
        for (Channel& c : p.channels) {
            if (iterFunc(c, counter)) return true;
            counter++;
        }
        return false;
    });
}


XPlug::Channel* XPlug::getChannelFromIndex(IPlugin* plug, size_t index)
{
    XPlug::Channel* channel =nullptr;
    iterateChannel(plug, [&channel, index](XPlug::Channel& c, size_t channelIndex) {
        if (index == channelIndex) {
            channel = &c;
            return true;
        }
        return false;
    });
    return channel;
}


