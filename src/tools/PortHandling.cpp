#include <tools/PortHandling.hpp>
using namespace XPlug;
size_t XPlug::getAudioChannelCount(IPlugin* plug, XPlug::PortDirection dir)
{
    size_t size = 0;
    iteratePorts<IAudioPort>(plug, dir, [&size](IAudioPort* p, size_t) { size += p->size(); return false;  });
    return size;
}

void XPlug::iterateAudioChannels(IPlugin* plug, std::function<bool(IAudioPort *, IAudioChannel*, size_t)> iterFunc)
{
    size_t counter = 0;
    iteratePorts<IAudioPort>(plug, [&counter, &iterFunc](IAudioPort* p, size_t index) {
        for (size_t i = 0; i < p->size(); i++) {
            if (iterFunc(p,p->at(i), counter)) return true;
            counter++;
        }
        return false;
        });
}
IAudioChannel* XPlug::getAudioChannelFromIndex(IPlugin* plug, size_t index)
{
    XPlug::IAudioChannel* channel = nullptr;
    iterateAudioChannels(plug, [&channel, index](IAudioPort* p,IAudioChannel* c, size_t channelIndex) {
        if (index == channelIndex) {
            channel = c;
            return true;
        }
        return false;
        });
    return channel;
}
void XPlug::iteratePortsFlat(IPlugin* plug, std::function<bool(XPlug::IPort*p, size_t ind)> iterFunc)
{
    size_t index = 0;
    for (int i = 0; i < plug->getPortComponent()->size(); i++) {
        auto aPort = dynamic_cast<IAudioPort*>(plug->getPortComponent()->at(i));
        if (aPort == nullptr) {
            if (iterFunc(plug->getPortComponent()->at(i), index++)) return;
        }
        else {
            for (int j = 0; j < aPort->size(); j++)
                if (iterFunc(plug->getPortComponent()->at(i), index++)) return;
        }
    }
}