#include <base/SimplePortComponent.hpp>
#include <Types.hpp>
using namespace XPlug;
XPlug::SimplePortComponent::SimplePortComponent(std::vector<Port> inputPorts, std::vector<Port> outputPorts)
{
    this->inputPorts = inputPorts;
    this->outputPorts = outputPorts;
}
size_t XPlug::SimplePortComponent::getNumberOfAllPorts()
{
    return getNumberOfInputPorts() + getNumberOfOutputPorts();
}

size_t XPlug::SimplePortComponent::getNumberOfInputPorts()
{
    return inputPorts.size();
}

size_t XPlug::SimplePortComponent::getNumberOfOutputPorts()
{
    return outputPorts.size();
}

 std::vector<Port>& XPlug::SimplePortComponent::getInputPorts()
{
    return inputPorts;
}

 std::vector<Port>& XPlug::SimplePortComponent::getOutputPorts()
{
    return outputPorts;
}

void XPlug::SimplePortComponent::addPort(Port p)
{
    if (p.direction == PortDirection::Input)
        this->inputPorts.push_back(p);
    else if (p.direction == PortDirection::Output)
        this->outputPorts.push_back(p);
    else
        throw NotImplementedException();
}
