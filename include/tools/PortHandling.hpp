#ifndef PORT_HANDLING_HPP
#define PORT_HANDLING_HPP
#include <interfaces/IPortComponent.hpp>
#include <interfaces/IPlugin.hpp>
#include <interfaces/IAudioPort.hpp>
#include <type_traits>
namespace XPlug {

    /**
     * @brief Iterate through all ports(first in and then output ports).
     * @param plug Plugin, which gives us the needed ports to iterate through
     * @param iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. (See implementation of \ref iterateChannel)
     *        The iterFunc takes a reference to a IPort, and the current Index of a IPort. If the function returns true, the iteration is abborted.
     */
    void iteratePorts(IPlugin* plug, std::function<bool(XPlug::IPort*, size_t)> iterFunc);

    /**
     * @brief Iterate through all Ports, which are type of the given template parameter and match the given PortDirection dir. Dynamic Casts can kill the RT Capability here. But not have to.
     * @tparam T Subtype of IPort, which is checked to match, while iterating Ports. So the given function is only triggered, if a Port can be casted to T. The Class name should not contain a Pointer.
     * @param plug Plugin, to iterate through
     * @param iterFunc Iterfunction, which is called every iteration, when the given parameters match the port.
     * @param dir Portdirection to filter for.
     */
    template<typename T>
    void iteratePortsFiltered(IPlugin* plug, XPlug::PortDirection dir, std::function<bool(T*, size_t)> iterFunc ) {
        static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
        size_t counter = 0;
        for (size_t i = 0; i < plug->getPortComponent()->size(); i++) {
            auto pl = dynamic_cast<T*>(plug->getPortComponent()->at(i));
            if (pl && pl->getDirection() == dir) {
                if (iterFunc(pl, counter)) return ;
                counter++;
            }
        }
    }
    // Version which does not filter for direction.
    template<typename T>
    void iteratePortsFiltered(IPlugin* plug, std::function<bool(T*, size_t)> iterFunc) {
        static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
        size_t counter = 0;
        for (size_t i = 0; i < plug->getPortComponent()->size(); i++) {
            auto pl = dynamic_cast<T*>(plug->getPortComponent()->at(i));
            if (pl) {
                if (iterFunc(pl, counter)) return ;
                counter++;
            }
        }
    }

    // Version which does not filter for direction.
    template<typename T>
    T* getPortAt(IPlugin* plug, size_t index) {
        static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
        T* res = nullptr;
        iteratePortsFiltered<T>(plug, [&res, index](T* p, size_t ind) {
            if (index == ind) {
                res = p;
                return true;
            }
            return false;
            });
        return res;
    }

    // Version which does not filter for direction.
    template<typename T>
    T* getPortAt(IPlugin* plug, size_t index, PortDirection dir) {
        static_assert(std::is_base_of<IPort, T>::value, "T must derive from IPort.");
        T* res=nullptr;
        iteratePortsFiltered<T>(plug, dir, [&res, index](T* p, size_t ind) {
            if (index == ind) {
                res = p;
                return true;
            }
            return false; });
        return res;
    }
   
    inline IAudioPort* getAudioPortAt(IPlugin* plug, size_t index, PortDirection dir) { return getPortAt<IAudioPort>(plug, index, dir); }
    inline IAudioPort* getAudioInputPortAt(IPlugin* plug, size_t index) { return getAudioPortAt(plug, index, PortDirection::Input); }
    inline IAudioPort* getAudioOutputPortAt(IPlugin* plug, size_t index) { return getAudioPortAt(plug, index, PortDirection::Output); }
   
    /**
     * @brief Gets the size of Ports, which are type of the given template parameter and match the given PortDirection dir. Dynamic Casts can kill the RT Capability here. But not have to.
     * @tparam T Subtype of IPort, which is checked to match, while iterating Ports. So the given function is only triggered, if a Port can be casted to T. The Class name should not contain a Pointer.
     * @param plug Plugin, to iterate through
     * @param dir Portdirection to filter for. 
     * @return size of the matched Ports.
    */
    template<typename T>
    size_t getNumberOfPorts(IPlugin* plug, XPlug::PortDirection dir) {
        size_t size=0;
        iteratePortsFiltered<T>(plug, dir, [&size](T* port, size_t index) { size++; return false; });
        return size;
    }


    size_t getAudioChannelCount(IPlugin* plug, XPlug::PortDirection dir);
    size_t getAudioChannelCount(IPlugin* plug);

    void iterateAudioChannels(IPlugin* plug, std::function<bool(XPlug::IAudioChannel*, size_t)> iterFunc);

    /**
     * @brief
     * @param plug
     * @param index
     * @return
     */
    IAudioChannel* getAudioChannelFromIndex(IPlugin* plug, size_t index);


    class PortComponentCacheImpl;

    /**
     * @brief Class for speeding up the use of the IPortComponent, which may not be rly performant. 
     * This class may change in future to support something like IPortComponentCacheable or sth.
     * This class provides a way, to call methods for getting specific midi audio in and out, without the need to iterate much.
     * This class cant exists without given IPortComponent, so make sure if you use this class, that it does not exists anymore, when the component also wont exists.
     */
    class PortComponentCache {
    public:
        PortComponentCache(IPortComponent* c);
        IAudioPort* getAudioInPortAt(size_t index);
        IAudioPort* getAudioOutPortAt(size_t index);
        IMidiPort* getMidiInPortAt(size_t index);
        IMidiPort* getMidiOutPortAt(size_t index);

        size_t sizeAudioInPorts();
        size_t sizeAudioOutPorts();
        size_t sizeMidiInPorts();
        size_t sizeMidiOutPorts();
    private:
        std::unique_ptr<PortComponentCacheImpl> impl;
    };
}
#endif //! PORT_HANDLING_HPP