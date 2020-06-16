#ifndef SIMPLE_PORT_COMPONENT_HPP
#define SIMPLE_PORT_COMPONENT_HPP
#include <interfaces/IPortComponent.hpp>
namespace XPlug {
    /**
     * @brief Simple implementation from PortComponent. Ports are stored in 2 vectors. 
     * The Order is ordered by input and output ports, where the input ports come first.
     * The order is same as the call of addPort or the inputelements of the constructor, except that they are sorted for input and outputelements.
     */
    class DynamicPortComponent :public IPortComponent {
    public:
        DynamicPortComponent() = default;
        DynamicPortComponent(std::vector<std::unique_ptr<IPort>> inputPorts, std::vector<std::unique_ptr<IPort>> outputPorts);
        // Geerbt über IPortComponent
        virtual size_t size() override;
        virtual size_t sizeInputPorts() override;
        virtual size_t sizeOutputPorts() override;
        virtual IPort* at(size_t index) override;
        virtual IPort* inputPortAt(size_t index) override;
        virtual IPort* outputPortAt(size_t index) override;
        virtual void addPort(std::unique_ptr<IPort> p);
    private:
        std::vector<std::unique_ptr<IPort>> inputPorts;
        std::vector<std::unique_ptr<IPort>> outputPorts;

    };
}


#endif //! SIMPLE_PORT_COMPONENT_HPP