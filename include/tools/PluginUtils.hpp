#ifndef PLUGIN_UTILS_HPP
#define PLUGIN_UTILS_HPP
#include <interfaces/IPlugin.hpp>
#include <functional>
namespace XPlug {
    /**
     * @brief Get all channels from all input ports.
     * @param plug Plugin, to use. Every inputport from this plugin is triggered.
     * @return Sum of all channels from all inputports from the given plugin.
     */
    size_t getChannelInCount(IPlugin* plug);
    /**
     * @brief Get all channels from all output ports.
     * @param plug Plugin, to use. Every outputport from this plugin is triggered.
     * @return Sum of all channels from all outputports from the given plugin.
     */
    size_t getChannelOutCount(IPlugin* plug);

    /**
     * @brief Get all channels from all ports (input and output).
     * @param plug Plugin, to use. Every outputport from this plugin is triggered.
     * @return Sum of all channels from all ports from the given plugin.
     */
    size_t getChannelCount(IPlugin* plug);

    /**
     * @brief Iterate through all ports(first in and then output ports).
     * @param plug Plugin, which gives us the needed ports to iterate through
     * @param iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. (See implementation of \ref iterateChannel)
     *        The iterFunc takes a reference to a Port, and the current Index of a Port. If the function returns true, the iteration is abborted.
     */
    void iteratePorts(IPlugin* plug, std::function<bool (XPlug::Port&, size_t)> iterFunc);

    /**
     * @brief Iterate through all channels(first channels from in and then from output ports).
     * @param plug Plugin, which gives us the needed channels to iterate through
     * @param   iterFunc Function, which is called in every iteration process. No Raw fucntionptr, so Lambdas with bounded Values can be used. 
                The iterFunc takes a reference to a Channel, and the current Index of a Port. If the function returns true, the iteration is abborted.
      */
    void iterateChannel(IPlugin* plug, std::function<bool(XPlug::Channel&, size_t)> iterFunc);

    /**
     * @brief 
     * @param plug 
     * @param index 
     * @return 
     */
    Channel* getChannelFromIndex(IPlugin* plug, size_t index);

}

#endif //! PLUGIN_UTILS_HPP