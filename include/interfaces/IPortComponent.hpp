#ifndef I_PORT_COMPONENT_HPP
#define I_PORT_COMPONENT_HPP
#include <string>
#include <vector>
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
    Audio, // Normal Audio Port
    MIDI, // MidiPort
};

struct Channel {
public:
    ///  Channel() = default;
    //  Channel(const Channel&) = delete; // Remove CopyConstructor. This data should not been copied for performance resons.

    /**
         * @brief Role of Channel, like Midichannel 15 or LeftSpeaker.
         */
    uint64_t channelRole;

    /**
         * @brief maybesupport a cool name here? Not yet supported.
         */
    std::string name;

    /**
         * @brief 32 bit data. Currently represented as normal float.
         */
    float* data32;

    /**
         * @brief 64 bit data. Currently represented as normal double.
         */
    double* data64;
};
/**
     * @brief Representation of in and outputs of an plugin. An Port has multiple Channels.
     */
struct Port {
public:
    //Port() = default;
    //  Port(const Port&) = delete;// Remove CopyConstructor. This data should not been copied for performance resons.

    /**
         * @brief  Indicates the Port with a given Name.
         */
    std::string name;

    /**
         * @brief Indicates, what is the Role of the Port.
         */
    PortType type;

    /**
         * @brief Direction of Port(in or out)
         */
    PortDirection direction;

    /**
         * @brief Size of samples for each Channel.
         */
    size_t sampleSize;

    /**
         * @brief Container for all different Channels.
         */
    std::vector<Channel> channels;
};

class IPortComponent {
public:
    virtual size_t getNumberOfAllPorts() = 0;
    virtual size_t getNumberOfInputPorts() = 0;
    virtual size_t getNumberOfOutputPorts() = 0;

    virtual std::vector<Port>& getInputPorts() = 0;
    virtual std::vector<Port>& getOutputPorts() = 0;
};
}

#endif //! I_PORT_COMPONENT_HPP
