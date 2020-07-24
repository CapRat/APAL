#ifndef IAUDIO_PORT_HPP
#define IAUDIO_PORT_HPP
#include "IPort.hpp"
#include "Speaker.hpp"
namespace XPlug {
    /**
     * @brief Class, which implements AudioChannel
     */
    class IAudioChannel {
    public:
        /**
         * @brief Virtual distructor, so implementing classes can also be destroyed correctly.
         */
        virtual ~IAudioChannel() = default;
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
        virtual void feed(float* data32) = 0;

        /**
         * @brief Typesafe implementation of \ref IChannel::get.
         * @param data typesafe param
         */
        virtual float* getData() = 0;
      //  virtual double* getData64() = 0;
    };



    /**
     * @brief
     */
    class IAudioPort :public IPort {
    public:
        /**
         * @brief Virtual distructor, so implementing classes can also be destroyed correctly.
         */
        virtual ~IAudioPort() = default;
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


        /**
         * @brief Typesafe implementation of at. Must have this strange name, because you cant overlaod functions, which just differ in returntype.
         * @param index
         * @return
        */
        virtual IAudioChannel* at(size_t index) = 0;
        /**
         * @brief returns the numbers of channels in this Port.
         * @return
         */
        virtual size_t size() = 0;


    };
}

#endif //! IAUDIO_PORT_HPP