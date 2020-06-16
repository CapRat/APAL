#include <tools/PluginUtils.hpp>
using namespace XPlug;

size_t XPlug::getChannelInCount(IPlugin* plug) {
    size_t pCount = 0;
    for (int i = 0; i < plug->getPortComponent()->sizeInputPorts(); i++) {
        if (plug->getPortComponent()->inputPortAt(i)->getType() == PortType::Audio) {
            pCount += plug->getPortComponent()->inputPortAt(i)->size();
        }
        pCount += plug->getPortComponent()->inputPortAt(i)->size();
    }
    return pCount;
}

size_t XPlug::getChannelOutCount(IPlugin* plug) {
    size_t pCount = 0;
    for (int i = 0; i < plug->getPortComponent()->sizeOutputPorts(); i++) {
        pCount += plug->getPortComponent()->inputPortAt(i)->size();
    }
    return pCount;
}

size_t XPlug::getChannelCount(IPlugin* plug)
{
    size_t pCount = 0;
    for (int i = 0; i < plug->getPortComponent()->size(); i++) {
        pCount += plug->getPortComponent()->at(i)->size();
    }
    return pCount;
}

void XPlug::iteratePorts(IPlugin* plug, std::function<bool(XPlug::IPort*, size_t)> iterFunc)
{
    for (size_t i = 0; i < plug->getPortComponent()->size();i++) {
        if (iterFunc(plug->getPortComponent()->at(i), i))return;
    }
}
void XPlug::iterateAudioPorts(IPlugin* plug, std::function<bool(XPlug::IAudioPort*, size_t)> iterFunc) {
    iteratePorts(plug, [iterFunc](XPlug::IPort* p, size_t ind) {
        auto audPort= dynamic_cast<IAudioPort*>(p);
        if(audPort)
            if (iterFunc(audPort, ind))return true;
        return false;
        });
}
void XPlug::iterateAudioChannel(IPlugin* plug, std::function<bool(XPlug::IAudioChannel*, size_t)> iterFunc)
{
    size_t counter = 0;
    iterateAudioPorts(plug, [&counter,&iterFunc](IAudioPort* p, size_t index) {
        for (size_t i = 0; i < p->size(); i++) {
            if (iterFunc(p->typesafeAt(i), counter)) return true;
            counter++;
        }
        return false;
    });
}


XPlug::IAudioChannel* XPlug::getChannelFromIndex(IPlugin* plug, size_t index)
{
    XPlug::IAudioChannel* channel =nullptr;
    iterateAudioChannel(plug, [&channel, index](XPlug::IAudioChannel* c, size_t channelIndex) {
        if (index == channelIndex) {
            channel = c;
            return true;
        }
        return false;
    });
    return channel;
}
/*
bool XPlug::PortIterator::operator==(const iterator&, const iterator&)
{
    return false;
}

bool XPlug::operator==(const PortIterator&, const PortIterator&)
{
    return false;
}

bool XPlug::operator!=(const PortIterator&, const PortIterator&)
{
    return false;
}
PortIterator XPlug::PortIterator::operator++(int)
{
    return iterator();
}

PortIterator XPlug::PortIterator::operator*() const
{
    return value_type();
}

PortIterator: XPlug::PortIterator::operator->() const
{
    return pointer();
}
*/