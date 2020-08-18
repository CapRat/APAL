#ifndef I_MIDI_PORT_HPP
#define I_MIDI_PORT_HPP
#include "IPort.hpp"
namespace APAL {

/**
 * @brief Midi events as enum with correct Byte representation.
 */
enum class MidiEvents : uint8_t
{
  NoteOff = 0b1000 << 4,
  NoteOn = 0b1001 << 4,
  PolyphonicKeyPressure = 0b1010 << 4,
  ControlChange = 0b1011 << 4,
  ChannelModeMessage = ControlChange,
  ProgramChange = 0b1100 << 4,
  ChannelPressure = 0b1101 << 4,
  PitchBendChange = 0b1101 << 4,
};

typedef std::array<uint8_t, 3> MidiMessage;

/**
 * @brief IMidiPort derives from IPort and manages mutliple MidiEvents.
 */
class IMidiPort : public IPort
{
public:
  /**
   * @brief Virtual distructor, so implementing classes can also be destroyed
   * correctly.
   */
  virtual ~IMidiPort() = default;
  /**
   * @brief Adds an MidiMessage to the Ports, which can be stored. The
   * MidiMessage is moved, so the container takes responsibility.
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
#endif //! I_MIDI_PORT_HPP