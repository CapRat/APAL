#include <base/Ports.hpp>
#include <queue>
using namespace XPlug;

AudioChannel::AudioChannel(std::string _channelname, SpeakerPosition _pos)
{
  this->channelname = _channelname;
  this->pos = _pos;
  this->data = nullptr;
}
// Geerbt über IAudioChannel
std::string_view
AudioChannel::getName()
{
  return this->channelname;
}
void
AudioChannel::setRole(SpeakerPosition _pos)
{
  this->pos = _pos;
}
SpeakerPosition
AudioChannel::getRole()
{
  return this->pos;
}
void
AudioChannel::feed(float* _data)
{
  this->data = _data;
}

float*
XPlug::AudioChannel::getData()
{
  return this->data;
}

/*struct XPlug::DynamicAudioPort::impl {
    std::string name;
    std::vector<std::unique_ptr<IAudioChannel>> channels;
    SpeakerConfiguration configData;
};
XPlug::DynamicAudioPort::DynamicAudioPort(std::string name, PortDirection dir,
std::vector<std::unique_ptr<IAudioChannel>> channels) :
BasePort(std::move(name), dir)
{
    this->pImpl->configData = SpeakerConfiguration::Undefined;
    this->pImpl->channels = std::move(channels);

    for (int i = 0; i < this->pImpl->channels.size(); i++) {
        this->pImpl->configData =
static_cast<SpeakerConfiguration>(this->pImpl->configData |
this->pImpl->channels[i]->getRole());
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
}*/

XPlug::QueueMidiPort::QueueMidiPort(std::string _name, PortDirection _dir)
{
  this->name = _name;
  this->dir = _dir;
}

std::string_view
XPlug::QueueMidiPort::getPortName()
{
  return std::string_view(this->name.c_str(), this->name.size());
}

inline PortType
XPlug::QueueMidiPort::getType()
{
  return PortType::MIDI;
}

PortDirection
XPlug::QueueMidiPort::getDirection()
{
  return this->dir;
}

void
XPlug::QueueMidiPort::feed(MidiMessage&& msg)
{
  this->midiMsgPipe.push(msg);
}

MidiMessage
XPlug::QueueMidiPort::peek()
{
  return this->midiMsgPipe.front();
}

MidiMessage
XPlug::QueueMidiPort::get()
{
  auto msg = this->midiMsgPipe.front();
  this->midiMsgPipe.pop();
  return msg;
}

bool
XPlug::QueueMidiPort::empty()
{
  return this->midiMsgPipe.empty();
}

size_t
XPlug::QueueMidiPort::size()
{
  return this->midiMsgPipe.size();
}
