#include <tools/PortHandling.hpp>
using namespace XPlug;

void XPlug::iteratePorts(IPlugin* plug, std::function<bool(XPlug::IPort*, size_t)> iterFunc) {
    for (size_t i = 0; i < plug->getPortComponent()->size(); i++) {
        if (iterFunc(plug->getPortComponent()->at(i), i))return;
    }
}



size_t XPlug::getAudioChannelCount(IPlugin* plug, XPlug::PortDirection dir) {
    size_t size=0;
    iteratePortsFiltered<IAudioPort>(plug, dir, [&size](IAudioPort* p, size_t) { size += p->size(); return false;  });
    return size;
}
size_t XPlug::getAudioChannelCount(IPlugin* plug) {
    size_t size = 0;
    iteratePortsFiltered<IAudioPort>(plug, [&size](IAudioPort* p, size_t) { size += p->size(); return false;  });
    return size;
}
void XPlug::iterateAudioChannels(IPlugin* plug, std::function<bool(XPlug::IAudioChannel*, size_t)> iterFunc)
{
    size_t counter = 0;
    iteratePortsFiltered<IAudioPort>(plug, [&counter, &iterFunc](IAudioPort* p, size_t index) {
        for (size_t i = 0; i < p->size(); i++) {
            if (iterFunc(p->at(i), counter)) return true;
            counter++;
        }
        return false;
        });
}


IAudioChannel* XPlug::getAudioChannelFromIndex(IPlugin* plug, size_t index) {
    XPlug::IAudioChannel* channel = nullptr;
    iterateAudioChannels(plug, [&channel, index](XPlug::IAudioChannel* c, size_t channelIndex) {
        if (index == channelIndex) {
            channel = c;
            return true;
        }
        return false;
        });
    return channel;
}


class PortComponentCacheImpl;