#include <base/Ports.hpp>

using namespace XPlug;

AudioChannel::AudioChannel(std::string channelname, SpeakerPosition pos) {
    this->channelname = channelname;
    this->pos = pos;
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
void AudioChannel::feed(AudioChannelData data)
{
    this->data.data32 = data.data32;
    this->data.data64 = data.data64;
}
 AudioChannel::AudioChannelData AudioChannel::get() 
{
     return this->data;
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

XPlug::DynamicAudioPort::DynamicAudioPort(std::string name, PortType type, PortDirection dir, std::vector<std::unique_ptr<IAudioChannel>> channels) : BasePort(std::move(name),type,dir)
{
    configData = SpeakerConfiguration::Undefined;
    this->channels = std::move(channels);
    
    for (int i = 0; i < this->channels.size();i++) {
        this->configData =static_cast<SpeakerConfiguration>( static_cast<uint64_t>(this->configData) | this->channels[i]->getRole());
    }
}

inline size_t XPlug::DynamicAudioPort::size()
{
    return this->channels.size();
}
inline IAudioChannel* XPlug::DynamicAudioPort::typesafeAt(size_t index) {
    return this->channels[index].get();
}

SpeakerConfiguration XPlug::DynamicAudioPort::getConfig()
{
    return configData;
}


