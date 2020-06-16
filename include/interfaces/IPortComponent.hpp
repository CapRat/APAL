#ifndef I_PORT_COMPONENT_HPP
#define I_PORT_COMPONENT_HPP

#include "IPort.hpp"
namespace XPlug {

    /**
     * @brief Component for Porthandling. This Interface and the implementation handles multiple IPorts.
     * The creation and adding ports, is handled by the implementation.
     * Virtually this Component holds 3 Arrays. The Input, outputs and inandoutputs ports. The meaning of the indexes is implemntationdependent.
     * So look there, if you want to know, which indecies are meaningful for which port.
     */
    class IPortComponent {
    public:


        /**
         * @brief Size of all Ports, included in the component
         * @return number of all stored Ports in this component.
         */
        virtual size_t size() = 0;

        /**
         * @brief gets the portelement with given index.
         * @param index index of port. Meaning is implementationdependent. Index could not be equal or higher than \ref size().
         * @return pointer to IPort implementation.
         */
        virtual IPort* at(size_t index) = 0;

        /**
         * @brief  Size of all input port in this component.
         * @return number of all input ports.
         */
        virtual size_t sizeInputPorts() = 0;

        /**
         * @brief gets the input portelement with given index.
         * @param index Index of port. Meaning is implementationdependent. Index could not be equal or higher than \ref sizeInputPorts().
         * @return pointer to IPort implementation.
         */
        virtual IPort* inputPortAt(size_t index) = 0;

        /**
         * @brief  Size of all output port in this component.
         * @return number of all output ports.
         */
        virtual size_t sizeOutputPorts() = 0;

        /**
         * @brief gets the output portelement with given index.
         * @param index Index of port. Meaning is implementationdependent. Index could not be equal or higher than \ref sizeOutputPorts().
         * @return pointer to IPort implementation.
         */
        virtual IPort* outputPortAt(size_t index) = 0;
    };
}

#endif //! I_PORT_COMPONENT_HPP
