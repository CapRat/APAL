#ifndef IPORT_HPP
#define IPORT_HPP
#include <Types.hpp>
namespace XPlug {
    enum class PortDirection {
        Input,
        Output
        // Sidechain
    };

    /**
     * @brief Types of Ports
     */
    enum class PortType {
        None, //Why ever u want to do this.
        Audio, // Normal Audio IPort
        MIDI, // MidiPort
    };

    /**
     * @brief Feeds Data to the Channel. Should just be called from format implementations. See \ref IAudioChannel for more data.
     * @param data data to feed. If 64 bitprocessing is supported, it can be castet to double*. If not, it can be castet to float*
     */
    class IChannel {
    public:
        /**
         * @brief Inputs Data. The Data is a void ptr, wich can be cast to corresponding Type
         * @param data 
         */
        virtual void feed(void* data)=0;

        /**
         * @brief  Set given DataPtr to the requestet return value.
         * @param getData 
         */
        virtual void get(void* getData) = 0;
    };


    /**
     * @brief Representation of in and outputs of an plugin. An IPort has multiple Channels, which can be accessed though at.
     * In Future there could be added iteratorinterfaces.
     */
    class IPort {
    public:
        /**
         * @brief Indicates the IPort with a given Name.
         * @return the name of the IPort
         */
        virtual std::string_view getPortName() = 0;

        /**
         * @brief Indicates, what is the Role of the IPort. see \ref PortType for possible values.
         * @return Enum type, which represents the role of the port.
         */
        virtual PortType getType() = 0;

        /**
         * @brief Indicates, which Direction of IPort(in or out) is used. See \ref PortDirection for possible Values.
         * @return  Enum type, which represents the direction of the port.
         */
        virtual PortDirection getDirection() = 0;
        /**
         * @brief returns the numbers of channels in this Port.
         * @return
         */
        virtual size_t size() = 0;

        /**
         * @brief Gets the Channel on given index.
         * @param index Index of Channel. Meaning is implementationdependent. Index could not be equal or higher than \ref size().
         * @return Pointer to Channel on given index.
         */
        virtual IChannel* at(size_t index) = 0;
    };
}


#endif //! IPORT_HPP