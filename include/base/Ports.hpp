#ifndef PORTS_HPP
#define PORTS_HPP
#include <interfaces/Ports/IAudioPort.hpp>
#include <interfaces/Ports/IMidiPort.hpp>
#include <interfaces/Ports/IPortComponent.hpp>
#include <memory>
#include <queue>
#include <tools/PortHandling.hpp>
/******************************Some Port
 * definitions******************************/
namespace XPlug {

class AudioChannel : public IAudioChannel
{
public:
  AudioChannel(std::string _channelname, SpeakerPosition _pos);
  // Geerbt über IAudioChannel
  virtual std::string_view getName() override;
  virtual void setRole(SpeakerPosition _pos) override;
  virtual SpeakerPosition getRole() override;
  virtual void feed(float* data) override;
  virtual float* getData() override;

private:
  float* data;

  std::string channelname;
  SpeakerPosition pos;
};

template<int number_of_channels, SpeakerConfiguration configuration>
class StaticAudioPort : public IAudioPort
{
protected:
  std::array<std::unique_ptr<IAudioChannel>, number_of_channels> channels;
  std::string name;
  PortDirection dir;
  size_t sampleSize;

public:
  StaticAudioPort(std::string _name, PortDirection _dir)
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
  inline virtual size_t getSampleSize() override { return this->sampleSize; }
  inline virtual void setSampleSize(size_t _sampleSize) override
  {
    this->sampleSize = _sampleSize;
  }
};

typedef StaticAudioPort<1, SpeakerConfiguration::Mono> MonoPort;
typedef StaticAudioPort<2, SpeakerConfiguration::Stereo2_0> StereoPort;
typedef StaticAudioPort<3, SpeakerConfiguration::Stereo2_1>
  StereoWithSubwooferPort;
typedef StaticAudioPort<5, SpeakerConfiguration::Surround5_0> Surround5_0Port;
typedef StaticAudioPort<6, SpeakerConfiguration::Surround5_1> Surround5_1Port;

class QueueMidiPort : public IMidiPort
{
protected:
  std::string name;
  PortDirection dir;
  std::queue<MidiMessage> midiMsgPipe;

public:
  QueueMidiPort(std::string name, PortDirection dir);
  // Geerbt über IMidiPort
  virtual std::string_view getPortName() override;
  virtual PortType getType() override;
  virtual PortDirection getDirection() override;
  virtual void feed(MidiMessage&& msg) override;
  virtual MidiMessage peek() override;
  virtual MidiMessage get() override;
  virtual bool empty() override;
  virtual size_t size() override;
};
}

#endif //! PORTS_HPP