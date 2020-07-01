#include <base/Ports.hpp>
#include <queue>
using namespace XPlug;

AudioChannel::AudioChannel(std::string channelname, SpeakerPosition pos) {
    this->channelname = channelname;
    this->pos = pos;
    this->data32 = nullptr;
    this->data64 = nullptr;
}
// Geerbt über IAudioChannel
std::string_view AudioChannel::getName() 
{
    return this->channelname;
}
void AudioChannel::setRole(SpeakerPosition pos)
{
    this->pos = pos;
}
 SpeakerPosition AudioChannel::getRole()
{
     return this->pos;
}
void AudioChannel::feed(float* data32, double* data64)
{
    this->data32 = data32;
    this->data64 = data64;
}

 float* XPlug::AudioChannel::getData32()
 {
     return this->data32;
 }
 double* XPlug::AudioChannel::getData64()
 {
     return this->data64;
 }
XPlug::BasePort::BasePort(std::string name, PortType type, PortDirection dir )
{
    this->name = std::move(name);
    this->type = type;
    this->dir = dir;
    this->sampleSize = 0;
}

inline std::string_view XPlug::BasePort::getPortName()
{
    return std::string_view(this->name.c_str(), this->name.size());
}

inline PortType XPlug::BasePort::getType()
{
    return this->type;
}

inline PortDirection XPlug::BasePort::getDirection()
{
    return this->dir;
}

inline size_t XPlug::BasePort::getSampleSize()
{
    return this->sampleSize;
}

inline void XPlug::BasePort::setSampleSize(size_t sampleSize)
{
    this->sampleSize = sampleSize;
}
struct XPlug::DynamicAudioPort::impl {
    std::string name;
    std::vector<std::unique_ptr<IAudioChannel>> channels;
    SpeakerConfiguration configData;
};
XPlug::DynamicAudioPort::DynamicAudioPort(std::string name, PortType type, PortDirection dir, std::vector<std::unique_ptr<IAudioChannel>> channels) : BasePort(std::move(name), type, dir)
{
    this->pImpl->configData = SpeakerConfiguration::Undefined;
    this->pImpl->channels = std::move(channels);

    for (int i = 0; i < this->pImpl->channels.size(); i++) {
        this->pImpl->configData = static_cast<SpeakerConfiguration>(this->pImpl->configData | this->pImpl->channels[i]->getRole());
    }
}

inline size_t XPlug::DynamicAudioPort::size()
{
    return this->pImpl->channels.size();
}
inline IAudioChannel* XPlug::DynamicAudioPort::at(size_t index) {
    return this->pImpl->channels[index].get();
}

SpeakerConfiguration XPlug::DynamicAudioPort::getConfig()
{
    return pImpl->configData;
}


XPlug::QueueMidiPort::QueueMidiPort(std::string name, PortType type, PortDirection dir)
{
    this->name = name;
    this->type = type;
    this->dir = dir;
}

std::string_view XPlug::QueueMidiPort::getPortName()
{
    return std::string_view(this->name.c_str(), this->name.size());
}

PortType XPlug::QueueMidiPort::getType()
{
    return this->type;
}

PortDirection XPlug::QueueMidiPort::getDirection()
{
    return this->dir;
}

void XPlug::QueueMidiPort::feed(MidiMessage&& msg)
{
    this->midiMsgPipe.push(msg);
}

MidiMessage XPlug::QueueMidiPort::peek()
{
    return  this->midiMsgPipe.front();
}

MidiMessage XPlug::QueueMidiPort::get()
{
    auto msg = this->midiMsgPipe.front();
    this->midiMsgPipe.pop();
    return msg;
}

bool XPlug::QueueMidiPort::empty()
{
    return this->midiMsgPipe.empty();
}

size_t XPlug::QueueMidiPort::size()
{
    return this->midiMsgPipe.size();
}
