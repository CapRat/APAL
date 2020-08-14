#ifndef PORTS_HPP
#define PORTS_HPP
#include <interfaces/Ports/IAudioPort.hpp>
#include <interfaces/Ports/IMidiPort.hpp>
#include <interfaces/Ports/IPortComponent.hpp>
#include <memory>
#include <queue>
#include <tools/PortHandling.hpp>
/*************************Some Port definitions*************************/
namespace XPlug {

/**
 * @brief Standard implementation of a channel. Pretty straight forward.
 */
class AudioChannel : public IAudioChannel
{
public:
  inline AudioChannel(std::string _channelname, SpeakerPosition _pos)
    : channelname(_channelname)
    , pos(_pos)
    , data(nullptr)
  {}
  // Geerbt über IAudioChannel
  inline virtual std::string_view getName() override
  {
    return this->channelname;
  }
  inline virtual void setRole(SpeakerPosition _pos) override
  {
    this->pos = _pos;
  }
  inline virtual SpeakerPosition getRole() override { return this->pos; }
  inline virtual void feed(float* _data) override { this->data = _data; }
  inline virtual float* getData() override { return this->data; }

private:
  float* data;

  std::string channelname;
  SpeakerPosition pos;
};

/**
 * @brief Implementation of an AudioPort with static numbers of channel. Its
 * implemented through the use of an std::array. This class should be pretty
 * performant, so use this if you can. (A dynamic implementation doesnt exist
 * soo... just use this ;)  )
 */
template<int number_of_channels, SpeakerConfiguration configuration>
class StaticAudioPort : public IAudioPort
{
protected:
  std::array<std::unique_ptr<IAudioChannel>, number_of_channels> channels;
  std::string name;
  PortDirection dir;
  size_t sampleSize;

public:
  /**
   * @brief Constructor, which iterates one time over all channels. So Creation
   * is not that cheap. But with this implementation, the channels are created
   * pretty complete and the user desnt have something todo. If this is rly an
   * performance issue than... create everything static
   * @param _name Name of audioport.
   * @param _dir direction, which should be in or out at the moment.
   */
  inline StaticAudioPort(std::string _name, PortDirection _dir)
  {
    this->name = std::move(_name);
    this->dir = _dir;
    for (int i = 0; i < number_of_channels; i++)
      this->channels[i] = std::make_unique<AudioChannel>(
        this->name + getSpeakerSuffix<configuration>(i),
        getSpeakerPositionAt<configuration>(i));
  }

  // Geerbt über IAudioPort
  inline virtual size_t size() override { return number_of_channels; }
  inline virtual IAudioChannel* at(size_t index) override
  {
    return this->channels[index].get();
  }
  inline virtual SpeakerConfiguration getConfig() override
  {
    return configuration;
  }
  inline virtual std::string_view getPortName() override
  {
    return std::string_view(this->name.c_str(), this->name.size());
  }
  inline virtual PortType getType() override { return PortType::Audio; }
  inline virtual PortDirection getDirection() override { return this->dir; }
  inline virtual size_t getSampleCount() override { return this->sampleSize; }
  inline virtual void setSampleCount(size_t _sampleSize) override
  {
    this->sampleSize = _sampleSize;
  }
};

/******************Typedef for important AudioPorttypes*******************/
typedef StaticAudioPort<1, SpeakerConfiguration::Mono> MonoPort;
typedef StaticAudioPort<2, SpeakerConfiguration::Stereo2_0> StereoPort;
typedef StaticAudioPort<3, SpeakerConfiguration::Stereo2_1>
  StereoWithSubwooferPort;
typedef StaticAudioPort<5, SpeakerConfiguration::Surround5_0> Surround5_0Port;
typedef StaticAudioPort<6, SpeakerConfiguration::Surround5_1> Surround5_1Port;

/**
 * @brief Implementation of the IMidiPort interface with an std::queue.
 */
class QueueMidiPort : public IMidiPort
{
protected:
  std::string name;
  PortDirection dir;
  std::queue<MidiMessage> midiMsgPipe;

public:
  inline QueueMidiPort(std::string _name, PortDirection _dir)
    : name(_name)
    , dir(_dir)
  {}
  // Geerbt über IMidiPort
  inline virtual std::string_view getPortName() override
  {
    return std::string_view(this->name.c_str(), this->name.size());
  }
  inline virtual PortType getType() override { return PortType::MIDI; }
  inline virtual PortDirection getDirection() override { return this->dir; }
  inline virtual void feed(MidiMessage&& msg) override
  {
    this->midiMsgPipe.push(msg);
  }
  inline virtual MidiMessage peek() override
  {
    return this->midiMsgPipe.front();
  }
  inline virtual MidiMessage get() override
  {
    auto msg = this->midiMsgPipe.front();
    this->midiMsgPipe.pop();
    return msg;
  }
  inline virtual bool empty() override { return this->midiMsgPipe.empty(); }
  inline virtual size_t size() override { return this->midiMsgPipe.size(); }
};
}

#endif //! PORTS_HPP