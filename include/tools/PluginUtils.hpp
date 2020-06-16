#ifndef PLUGIN_UTILS_HPP
#define PLUGIN_UTILS_HPP
#include <interfaces/IPlugin.hpp>
#include <interfaces/IAudioPort.hpp>
#include <functional>
#include <iterator>
namespace XPlug {


    /**
     * @brief Get all channels from all input audio ports.
     * @param plug Plugin, to use. Every inputport from this plugin is triggered.
     * @return Sum of all channels from all inputports from the given plugin.
     */
    size_t getChannelInCount(IPlugin* plug);
    /**
     * @brief Get all channels from all output audio ports.
     * @param plug Plugin, to use. Every outputport from this plugin is triggered.
     * @return Sum of all channels from all outputports from the given plugin.
     */
    size_t getChannelOutCount(IPlugin* plug);

    /**
     * @brief Get all channels from all audio ports (input and output).
     * @param plug Plugin, to use. Every outputport from this plugin is triggered.
     * @return Sum of all channels from all ports from the given plugin.
     */
    size_t getChannelCount(IPlugin* plug);

    /**
     * @brief Iterate through all ports(first in and then output ports).
     * @param plug Plugin, which gives us the needed ports to iterate through
     * @param iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. (See implementation of \ref iterateChannel)
     *        The iterFunc takes a reference to a IPort, and the current Index of a IPort. If the function returns true, the iteration is abborted.
     */
    void iteratePorts(IPlugin* plug, std::function<bool (XPlug::IPort*, size_t)> iterFunc);

    /**
     * @brief Iterate through all the audioports(first in and then output ports).
     * @param plug Plugin, which gives us the needed ports to iterate through
     * @param iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. (See implementation of \ref iterateChannel)
     *        The iterFunc takes a reference to a IAudioPort, and the current Index of a IPort. If the function returns true, the iteration is abborted.
     */
    void iterateAudioPorts(IPlugin* plug, std::function<bool(XPlug::IAudioPort*, size_t)> iterFunc);

    /**
     * @brief Iterate through all channels(first channels from in and then from output ports).
     * @param plug Plugin, which gives us the needed channels to iterate through
     * @param   iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. 
                The iterFunc takes a reference to a Channel, and the current Index of a IPort. If the function returns true, the iteration is abborted.
      */
    void iterateAudioChannel(IPlugin* plug, std::function<bool(XPlug::IAudioChannel*, size_t)> iterFunc);

    /**
     * @brief 
     * @param plug 
     * @param index 
     * @return 
     */
    IAudioChannel* getChannelFromIndex(IPlugin* plug, size_t index);

}

#endif //! PLUGIN_UTILS_HPP