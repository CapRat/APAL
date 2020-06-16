#ifndef STATIC_PORT_COMPONENT
#define STATIC_PORT_COMPONENT
#include <interfaces/IPortComponent.hpp>
namespace XPlug {
    template<size_t inputPortsSize,size_t outputPortsSize>
    class StaticPortComponent :public IPortComponent {
    public:
        StaticPortComponent(std::array<std::unique_ptr<IPort>, inputPortsSize> ínputs, std::array<std::unique_ptr<IPort>, outputPortsSize> outputs) {
            this->inputPorts = std::move(inputs);
            this->outputPorts = std::move(outputs);
        }
        // Geerbt über IPortComponent
        inline virtual size_t size() override { return inputPortsSize+ outputPortsSize; }
        inline virtual IPort* at(size_t index) override { return index < inputPortsSize ? inputPorts[index].get() : outputPorts[index - inputPortsSize].get; }
        inline virtual size_t sizeInputPorts() override { return inputPortsSize; }
        inline virtual IPort* inputPortAt(size_t index) override { return inputPorts[index].get(); }
        inline virtual size_t sizeOutputPorts() override { return outputPortsSize; }
        inline virtual IPort* outputPortAt(size_t index) override { return outputPorts[index].get(); }
    protected:
        std::array<std::unique_ptr<IPort>, inputPortsSize> inputPorts;
        std::array<std::unique_ptr<IPort>, outputPortsSize> outputPorts;
    };
}

#endif //! STATIC_PORT_COMPONENT