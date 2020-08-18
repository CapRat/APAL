#ifndef PORT_HANDLING_HPP
#define PORT_HANDLING_HPP
#include <interfaces/IPlugin.hpp>
#include <interfaces/Ports/IAudioPort.hpp>
#include <interfaces/Ports/IMidiPort.hpp>
#include <interfaces/Ports/IPortComponent.hpp>
#include <type_traits>
namespace APAL {

/**
 * @brief Iterate through all Ports, which are type of the given template
 * parameter and match the given PortDirection dir. Dynamic Casts can kill the
 * RT Capability here. But not have to.
 * @tparam T Subtype of IPort, which is checked to match, while iterating Ports.
 * So the given function is only triggered, if a Port can be casted to T. The
 * Class name should not contain a Pointer.
 * @param plug Plugin, to iterate through
 * @param dir Portdirection to filter for.
 * @param iterFunc Iterfunction, which is called every iteration, when the given
 * parameters match the port.
 */
template<typename T>
void
iteratePorts(IPlugin* plug,
             APAL::PortDirection dir,
             std::function<bool(T*, size_t)> iterFunc)
{
  static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
  size_t counter = 0;
  for (size_t i = 0; i < plug->getPortComponent()->size(); i++) {
    auto pl = dynamic_cast<T*>(plug->getPortComponent()->at(i));
    if (pl && (dir == PortDirection::All || pl->getDirection() == dir)) {
      if (iterFunc(pl, counter))
        return;
      counter++;
    }
  }
}
/**
 * @brief Aliasfunction for iteratePorts, which doesnt filter the direction.
 * @tparam T Subtype of IPort, which is checked to match, while iterating Ports.
 * So the given function is only triggered, if a Port can be casted to T. The
 * Class name should not contain a Pointer.
 * @param plug  Plugin, to iterate through
 * @param iterFunc Iterfunction, which is called every iteration, when the given
 * parameters match the port.
 */
template<typename T>
inline void
iteratePorts(IPlugin* plug, std::function<bool(T*, size_t)> iterFunc)
{
  iteratePorts<T>(plug, APAL::PortDirection::All, iterFunc);
}

/**
 * @brief gets a Port at the given index. The filtering is done before, so its
 * not an absolute index.
 * @tparam T Type of Ports to be filtered.
 * @param plug plugin to get Ports from.
 * @param index index which match the wanted port.
 * @param dir filters the direction from the Port.
 * @return the matched port or nullptr if nothing is found.
 */
template<typename T>
T*
getPortAt(IPlugin* plug, size_t index, PortDirection dir = PortDirection::All)
{
  static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
  T* res = nullptr;
  iteratePorts<T>(plug, dir, [&res, index](T* p, size_t ind) {
    if (index == ind) {
      res = p;
      return true;
    }
    return false;
  });
  return res;
}

/**
 * @brief Gets the size of Ports, which are type of the given template parameter
 * and match the given PortDirection dir. Dynamic Casts can kill the RT
 * Capability here. But not have to.
 * @tparam T Subtype of IPort, which is checked to match, while iterating Ports.
 * So the given function is only triggered, if a Port can be casted to T. The
 * Class name should not contain a Pointer.
 * @param plug Plugin, to iterate through
 * @param dir Portdirection to filter for.
 * @return size of the matched Ports.
 */
template<typename T>
size_t
getNumberOfPorts(IPlugin* plug, APAL::PortDirection dir)
{
  size_t size = 0;
  iteratePorts<T>(plug, dir, [&size](T*, size_t) {
    size++;
    return false;
  });
  return size;
}

/**
 * @brief Retreives the size of all AudioChannels in a plugin.
 * @param plug   Plugin, to iterate through
 * @param dir Direction to filter Channelcount. Default is All, which means that
 * there is no filtering.
 * @return
 */
size_t
getAudioChannelCount(IPlugin* plug,
                     APAL::PortDirection dir = PortDirection::All);

/**
 * @brief Iterates all Audiochannels in a Plugin.
 * @param plug  Plugin, to iterate through
 * @param iterFunc terfunction, which is called every iteration, when the given
 * parameters match the channel.
 */
void
iterateAudioChannels(
  IPlugin* plug,
  std::function<bool(IAudioPort*, IAudioChannel*, size_t)> iterFunc);

/**
 * @brief Gets a channel with a given index. The index are the same, as in \see
 * iterateAudioChannels .
 * @param plug Plugin, to iterate through
 * @param index index to get channel from.
 * @return the channel at given index.
 */
IAudioChannel*
getAudioChannelFromIndex(IPlugin* plug, size_t index);

/**
 * @brief Iterates all the Ports, but iterates over Audioports, like they have
 * an Audioport for each Channel. Thisway Channel indexes can be mapped flat.
 * @param plug
 * @param iterFunc
 */
void
iteratePortsFlat(IPlugin* plug,
                 std::function<bool(APAL::IPort*, size_t)> iterFunc);

}
#endif //! PORT_HANDLING_HPP