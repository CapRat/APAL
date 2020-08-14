#ifndef IPORT_HPP
#define IPORT_HPP
#include <Types.hpp>
namespace XPlug {
enum class PortDirection
{
  // None = 0,
  Input = 1 << 0,
  Output = 1 << 1,
  // Bidirectional=1<<2,
  // Sidechain = 1<<3
  All = Input | Output // Value, which is just for
};

/**
 * @brief Types of Ports
 */
enum class PortType
{
  None,  // Why ever u want to do this.
  Audio, // Normal Audio IPort
  MIDI,  // MidiPort
};

/**
 * @brief Representation of in and outputs of an plugin. An IPort has multiple
 * Channels, which can be accessed through at. In Future there could be added
 * iteratorinterfaces or methods for optimized access. 
 */
class IPort
{
public:
  /**
   * @brief Virtual distructor, so implementing classes can also be destroyed
   * correctly.
   */
  virtual ~IPort() = default;

  /**
   * @brief Indicates the IPort with a given Name.
   * @return the name of the IPort
   */
  virtual std::string_view getPortName() = 0;

  /**
   * @brief Indicates, what is the Role of the IPort. see \ref PortType for
   * possible values.
   * @return Enum type, which represents the role of the port.
   */
  virtual PortType getType() = 0;

  /**
   * @brief Indicates, which Direction of IPort(in or out) is used. See \ref
   * PortDirection for possible Values.
   * @return  Enum type, which represents the direction of the port.
   */
  virtual PortDirection getDirection() = 0;
};
}

#endif //! IPORT_HPP