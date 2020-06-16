#ifndef PORTS_HPP
#define PORST_HPP
#include <interfaces/IPortComponent.hpp>
#include <interfaces/IAudioPort.hpp>
#include "Channel.hpp"
/******************************Some Port definitions******************************/
namespace XPlug {


    class AudioChannel :public IAudioChannel {
    public:

        AudioChannel(std::string channelname, SpeakerPosition pos);
        // Geerbt über IAudioChannel
        virtual std::string_view getName() override;
        virtual void setRole(SpeakerPosition pos) override;
        virtual SpeakerPosition getRole() override;
        virtual void feed(AudioChannelData data) override;
        virtual AudioChannelData get() override;

    private:
        AudioChannelData data;
        std::string channelname;
        SpeakerPosition pos;
    };

    /**
     * @brief  Abstract class, which implements everything, except the channelmanagement.
     */
    class BasePort :public IAudioPort {
    protected:
        std::string name;
        PortType type;
        PortDirection dir;
        size_t sampleSize;
    public:
        BasePort(std::string name, PortType type, PortDirection dir);
        // Geerbt über IPort
        virtual std::string_view getPortName() override;
        virtual PortType getType() override;
        virtual PortDirection getDirection() override;
        virtual size_t getSampleSize() override;
        virtual void setSampleSize(size_t sampleSize) override;
     
    };

    template<int number_of_channels, SpeakerConfiguration configuration>
    class StaticAudioPort :public BasePort{
    protected:
        std::array<std::unique_ptr<IAudioChannel>, number_of_channels> channels;
    public:
        StaticAudioPort(std::string name, PortType type, PortDirection dir) : BasePort(std::move(name), type, dir)
        {
            for (int i = 0; i < number_of_channels; i++) 
                channels[i] =std::make_unique<AudioChannel>(name + std::to_string(i), getSpeakerPositionAt<configuration>(i));
                
        }
        inline virtual size_t size()override  {   return number_of_channels; }
        inline  virtual IAudioChannel* typesafeAt(size_t index) override {  return this->channels[index].get();}
        inline virtual SpeakerConfiguration getConfig()override {  return configuration;}
    };
    typedef StaticAudioPort <1, SpeakerConfiguration::Mono> MonoPort;

  //  StaticAudioPort < 1, SpeakerConfiguration::Mono>
    class DynamicAudioPort :public BasePort {
    protected:
        std::string name;
        std::vector<std::unique_ptr<IAudioChannel>> channels;
        SpeakerConfiguration configData;
    public:

        /**
         * @brief Creates a new Port. The SpeakerConfiguration information, are retrained from channel information.
         * @param name 
         * @param type 
         * @param dir 
         * @param channels 
         * @return 
         */
        DynamicAudioPort(std::string name, PortType type, PortDirection dir, std::vector<std::unique_ptr<IAudioChannel>> channels);
        // Geerbt über IPort
        virtual size_t size() override;

        virtual IAudioChannel* typesafeAt(size_t index) override;
        virtual SpeakerConfiguration getConfig() override;
    };

}

#endif //!PORST_HPP