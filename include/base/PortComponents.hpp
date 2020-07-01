#ifndef SIMPLE_PORT_COMPONENT_HPP
#define SIMPLE_PORT_COMPONENT_HPP
#include <interfaces/IPortComponent.hpp>
#include <memory>
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
        virtual size_t sizeInputPorts() ;
        virtual size_t sizeOutputPorts() ;
        virtual IPort* at(size_t index) override;
        virtual IPort* inputPortAt(size_t index);
        virtual IPort* outputPortAt(size_t index) ;
        virtual void addPort(std::unique_ptr<IPort> p);
    private:
        std::vector<std::unique_ptr<IPort>> inputPorts;
        std::vector<std::unique_ptr<IPort>> outputPorts;

    };

    template<size_t inputPortsSize, size_t outputPortsSize>
    class StaticPortComponent :public IPortComponent {
    public:
        StaticPortComponent(std::array<std::unique_ptr<IPort>, inputPortsSize> inputs, std::array<std::unique_ptr<IPort>, outputPortsSize> outputs) {
            this->inputPorts = std::move(inputs);
            this->outputPorts = std::move(outputs);
        }
        // Geerbt über IPortComponent
        inline virtual size_t size() override { return inputPortsSize + outputPortsSize; }
        inline virtual IPort* at(size_t index) override { return index < inputPortsSize ? inputPorts[index].get() : outputPorts[index - inputPortsSize].get; }
        inline virtual size_t sizeInputPorts()  { return inputPortsSize; }
        inline virtual IPort* inputPortAt(size_t index)  { return inputPorts[index].get(); }
        inline virtual size_t sizeOutputPorts()  { return outputPortsSize; }
        inline virtual IPort* outputPortAt(size_t index)  { return outputPorts[index].get(); }
    protected:
        std::array<std::unique_ptr<IPort>, inputPortsSize> inputPorts;
        std::array<std::unique_ptr<IPort>, outputPortsSize> outputPorts;
    };
}


#endif //! SIMPLE_PORT_COMPONENT_HPP