#ifndef I_PORT_COMPONENT_HPP
#define I_PORT_COMPONENT_HPP

#include "IPort.hpp"
namespace XPlug {

/**
 * @brief Component for porthandling. This interface and the implementation
 * handles multiple IPorts. The creation and adding ports, is handled by the
 * implementation. Virtually this Component holds multiple Arrays. The input,
 * output and inandoutput ports for controller and audio data. The meaning of
 * the indexes is implemntationdependent. So look there, if you want to know,
 * which indecies are meaningful for which port.
 */
class IPortComponent
{
public:
  /**
   * @brief Size of all Ports, included in the component
   * @return number of all stored Ports in this component.
   */
  virtual size_t size() = 0;

  /**
   * @brief gets the portelement with given index. The Portelement should be
   * same for every index.
   * @param index index of port. Meaning is implementationdependent. Index could
   * not be equal or higher than \ref size().
   * @return pointer to IPort implementation.
   */
  virtual IPort* at(size_t index) = 0;
};
}

#endif //! I_PORT_COMPONENT_HPP
