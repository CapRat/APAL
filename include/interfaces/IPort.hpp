#ifndef IPORT_HPP
#define IPORT_HPP
#include <Types.hpp>
namespace XPlug {
    enum class PortDirection {
       // None = 0,
        Input = 1 << 0,
        Output=1<<1
        // Bidirectional=1<<2,
        // Sidechain = 1<<3
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
     * @brief Representation of in and outputs of an plugin. An IPort has multiple Channels, which can be accessed though at.
     * In Future there could be added iteratorinterfaces.
     */
    class IPort {
    public:

        /**
         * @brief Virtual distructor, so implementing classes can also be destroyed correctly.
         */
        virtual ~IPort() = default;

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
    };
    enum class MidiEvents :uint8_t {
        NoteOff = 0b1000 << 4,
        NoteOn = 0b1001 <<4,
        PolyphonicKeyPressure = 0b1010 << 4,
        ControlChange = 0b1011 << 4,
        ChannelModeMessage = ControlChange,
        ProgramChange = 0b1100 << 4,
        ChannelPressure = 0b1101 << 4,
        PitchBendChange = 0b1101 << 4,
    };
    typedef std::array<uint8_t,3> MidiMessage ;
   
    /**
     * @brief MidiPort, which manage mutliple MidiEvents.
    */
    class IMidiPort :public IPort {
    public:
        /**
         * @brief Virtual distructor, so implementing classes can also be destroyed correctly.
         */
        virtual ~IMidiPort() = default;
        /**
         * @brief Adds an MidiMessage to the Ports, which can be stored. The MidiMessage is moved, so the container takes responsibility.
         * @param msg  Message to store.
        */
        virtual void feed(MidiMessage&& msg) = 0;
        /**
         * @brief Gets the actual MidiMessage, but not removes them
         * @return MidiMessage to do things with.
        */
        virtual MidiMessage peek() = 0;

        /**
         * @brief Gets the actual MidiMessage and removes them from the Port.
         * @return removed MidiMessage
        */
        virtual MidiMessage get() = 0;

        /**
         * @brief signals, that no MidiMessage is present.
         * @return true, if pipe is empty, false if pipe is not empty.
        */
        virtual bool empty() = 0;

        /**
         * @brief gets the size of items in the pipe
         * @return  size of items in the pipe
        */
        virtual size_t size() = 0;
    };


}


#endif //! IPORT_HPP