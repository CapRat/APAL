#include <base/DynamicPortComponent.hpp>
#include <Types.hpp>
using namespace XPlug;
XPlug::DynamicPortComponent::DynamicPortComponent(std::vector<std::unique_ptr<IPort>> inputPorts, std::vector<std::unique_ptr<IPort>> outputPorts)
{
    this->inputPorts = std::move(inputPorts);
    this->outputPorts = std::move( outputPorts);
}
size_t XPlug::DynamicPortComponent::size()
{
    return sizeInputPorts() + sizeOutputPorts();
}

size_t XPlug::DynamicPortComponent::sizeInputPorts()
{
    return inputPorts.size();
}

size_t XPlug::DynamicPortComponent::sizeOutputPorts()
{
    return outputPorts.size();
}
void XPlug::DynamicPortComponent::addPort(std::unique_ptr<IPort> p){
    if (p->getDirection() == PortDirection::Input)
        this->inputPorts.push_back(std::move(p));
    else if (p->getDirection() == PortDirection::Output)
        this->outputPorts.push_back(std::move(p));
    else
        throw NotImplementedException();
}


IPort* XPlug::DynamicPortComponent::at(size_t index)
{
    if (index < this->sizeInputPorts())
        return inputPortAt(index);
    else
        return outputPortAt(index - this->sizeInputPorts());
}

IPort* XPlug::DynamicPortComponent::inputPortAt(size_t index)
{
    return this->inputPorts[index].get();
}

IPort* XPlug::DynamicPortComponent::outputPortAt(size_t index)
{
    return this->outputPorts[index].get();
}

