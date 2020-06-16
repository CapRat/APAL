#ifndef IAUDIO_PORT_HPP
#define IAUDIO_PORT_HPP
#include <interfaces/IPort.hpp>
namespace XPlug {



    /**
      * @brief Enum, which represents different Positions of speakers.
      */
    enum SpeakerPosition : uint64_t {
        Undefined =  0,
        Left = 1 << 0,
        Right = 1 << 1,
        Center = 1 << 2,
        RearLeft = 1 << 3,
        RearRight = 1 << 4,
    };

    enum class SpeakerConfiguration : uint64_t {
        Undefined = SpeakerPosition::Undefined,
        Mono = SpeakerPosition::Center,
        Stereo = SpeakerPosition::Left | SpeakerPosition::Right,
        Surround5_1 = Stereo | SpeakerPosition::Center | SpeakerPosition::RearLeft | SpeakerPosition::RearRight,
    };

    /**
     * @brief Class, which implements AudioChannel
     */
    class IAudioChannel :public IChannel {
    public:

        struct AudioChannelData {
            float* data32;
            double* data64;
        };

        /**
         * @brief Gets the name of the current Channel.
         * @return the name of the current Channel
         */
        virtual std::string_view getName() = 0;

        /**
         * @brief Sets the Role of the Speaker for current channel.
         * @param pos The Speaker Position for the current channel.
         */
        virtual void setRole(SpeakerPosition pos) = 0;
        virtual SpeakerPosition getRole() = 0;

        /**
         * @brief Typesafe implementation of \ref IChannel::feed.
         * @param data typesafe param
         */
        virtual void feed(AudioChannelData data) = 0;

        /**
         * @brief Typesafe implementation of \ref IChannel::get.
         * @param data typesafe param
         */
        virtual AudioChannelData get() = 0;

        // Geerbt über IChannel
        inline virtual void feed(void* data) final {feed(*static_cast<AudioChannelData*>(data)); }

        /**
         * @brief This Method implements, not typical for interface, an Method. But thats ok, because its just changes the Castoperator to make it typesafe.
         *          So this method is just like an typesafe version of the Interface \ref IChannel::get .
         * @param getData data to cast to void and forward to IChannel::feed(void*);
         */
        inline virtual void get(void* getData) final { 
            auto data = get();
            static_cast<AudioChannelData*>(getData)->data32 = data.data32;
            static_cast<AudioChannelData*>(getData)->data64 = data.data64;
        }
    };



    /**
     * @brief
     */
    class IAudioPort :public IPort {
    public:

        /**
         * @brief Get more detailed config data.
         * @return SpeakerConfiguration of the Audioport.
         * AudioPorts should return a Bitmask here, which can be casted to \ref PortConfig
         */
        virtual SpeakerConfiguration getConfig() = 0;

        /**
         * @brief  Gets the Size of samples for each Channel. (all channelbuffes should have the same size)
         * @return Size of samples for each Channel.
         */
        virtual size_t getSampleSize() = 0;

        /**
         * @brief  Sets the Size of samples for each Channel. (all channelbuffes should have the same size)
         * This should just called from implementations plugins side, while initalizing data.
         * @param sampleSize the new samplesize.
         */
        virtual void setSampleSize(size_t sampleSize) = 0;

        inline virtual IChannel* at(size_t index) final { return typesafeAt(index); }

        /**
         * @brief Typesafe implementation of at. Must have this strange name, because you cant overlaod functions, which just differ in returntype.
         * @param index
         * @return
        */
        virtual IAudioChannel* typesafeAt(size_t index) = 0;
    };
}

#endif //! IAUDIO_PORT_HPP