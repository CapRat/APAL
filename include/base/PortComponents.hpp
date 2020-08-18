#ifndef SIMPLE_PORT_COMPONENT_HPP
#define SIMPLE_PORT_COMPONENT_HPP
#include <interfaces/Ports/IPortComponent.hpp>
#include <memory>
namespace APAL {
/**
 * @brief Simple implementation from PortComponent. Ports are stored in 2
 * vectors. The Order is ordered by input and output ports, where the input
 * ports come first. The order is same as the call of addPort or the
 * inputelements of the constructor, except that they are sorted for input and
 * outputelements.
 */
class DynamicPortComponent : public IPortComponent
{
public:
  DynamicPortComponent() = default;
  inline DynamicPortComponent(
    std::initializer_list<std::shared_ptr<IPort>> usedPorts)
  {
    this->ports.insert(this->ports.end(), usedPorts.begin(), usedPorts.end());
  }
  // Geerbt über IPortComponent
  inline virtual size_t size() override { return ports.size(); };
  inline virtual IPort* at(size_t index) override { return ports[index].get(); }

  inline virtual void addPort(std::shared_ptr<IPort> p)
  {
    this->ports.push_back(std::move(p));
  };

private:
  std::vector<std::shared_ptr<IPort>> ports;
};

template<size_t numberOfPorts>
class StaticPortComponent : public IPortComponent
{
public:
  StaticPortComponent(std::initializer_list<std::shared_ptr<IPort>> usedPorts)
  {
    std::copy(usedPorts.begin(), usedPorts.end(), this->ports.begin());
  }
  // Geerbt über IPortComponent
  inline virtual size_t size() override { return numberOfPorts; }
  inline virtual IPort* at(size_t index) override { return ports[index].get(); }

protected:
  std::array<std::shared_ptr<IPort>, numberOfPorts> ports;
};
/*
template<size_t inputPortsSize, size_t outputPortsSize>
class StaticPortDirectionSplittedPortComponent : public IPortComponent
{
public:
  StaticPortComponent(
    std::array<std::unique_ptr<IPort>, inputPortsSize> inputs,
    std::array<std::unique_ptr<IPort>, outputPortsSize> outputs)
  {
    this->inputPorts = std::move(inputs);
    this->outputPorts = std::move(outputs);
  }
  // Geerbt über IPortComponent
  inline virtual size_t size() override
  {
    return inputPortsSize + outputPortsSize;
  }
  inline virtual IPort* at(size_t index) override
  {
    return index < inputPortsSize ? inputPorts[index].get()
                                  : outputPorts[index - inputPortsSize].get;
  }
  inline virtual size_t sizeInputPorts() { return inputPortsSize; }
  inline virtual IPort* inputPortAt(size_t index)
  {
    return inputPorts[index].get();
  }
  inline virtual size_t sizeOutputPorts() { return outputPortsSize; }
  inline virtual IPort* outputPortAt(size_t index)
  {
    return outputPorts[index].get();
  }
protected:
  std::array<std::unique_ptr<IPort>, inputPortsSize> inputPorts;
  std::array<std::unique_ptr<IPort>, outputPortsSize> outputPorts;
};*/
}

#endif //! SIMPLE_PORT_COMPONENT_HPP