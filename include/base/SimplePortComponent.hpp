#ifndef SIMPLE_PORT_COMPONENT_HPP
#define SIMPLE_PORT_COMPONENT_HPP
#include <interfaces/IPortComponent.hpp>
namespace XPlug {
    class SimplePortComponent :public IPortComponent {
    public:
        SimplePortComponent(std::vector<Port> inputPorts, std::vector<Port> outputPorts);
        SimplePortComponent()=default;
        // Geerbt über IPortComponent
        virtual size_t getNumberOfAllPorts() override;
        virtual size_t getNumberOfInputPorts() override;
        virtual size_t getNumberOfOutputPorts() override;
        virtual  std::vector<Port>& getInputPorts() override;
        virtual  std::vector<Port>& getOutputPorts() override;
        virtual void addPort(Port p);
    private:
        std::vector<Port> inputPorts;
        std::vector<Port> outputPorts;
    };
}


#endif //! SIMPLE_PORT_COMPONENT_HPP