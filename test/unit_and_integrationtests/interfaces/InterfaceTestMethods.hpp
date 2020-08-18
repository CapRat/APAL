/**
 * This file includes testfuncitons, which should test weather the Interface is
 * implemented correctly. Every interface has one function, where an interface
 * can be testet. Make sure to pass correct initialized interfaces, cause empty
 * interfaces can not be tested very well.
 */

#ifndef INTERFACE_TEST_METHODS_HPP
#define INTERFACE_TEST_METHODS_HPP
#include <interfaces/IPlugin.hpp>
#include <interfaces/Ports/IAudioPort.hpp>
#include <interfaces/Ports/IMidiPort.hpp>

/**
 * @brief Tests an IPlugin-interface.
 * @param plug IPlugin implementation, which should be tested.
 */
void
testIPlugin(APAL::IPlugin* plug);

/**
 * @brief Tests the IInfoComponent-interface
 * @param infoComp  Interface to test
 */
void
testIInfoComponent(APAL::IInfoComponent* infoComp);

/**
 * @brief Tests the IFeatureComponent-interface
 * @param featComp Interface to test
 */
void
testIFeatureComponent(APAL::IFeatureComponent* featComp);

/**
 * @brief Tests the IPortComponent-interface
 * @param portComp Interface to test
 */
void
testIPortComponent(APAL::IPortComponent* portComp);

/**
 * @brief Tests an IPort for its Interfacemethods.
 * @param port Implementation to test.
 */
void
testIPort(APAL::IPort* port);

/**
 * @brief Tests an IAudioPort for its Interfacemethods.
 * @param aPort Implementation to test.
 */
void
testIAudioPort(APAL::IAudioPort* aPort);

/**
 * @brief Tests an IMidiPort for its Interfacemethods.
 * @param mPort Implementation to test.
 */
void
testIMidiPort(APAL::IMidiPort* mPort);

#endif //! INTERFACE_TEST_METHODS_HPP