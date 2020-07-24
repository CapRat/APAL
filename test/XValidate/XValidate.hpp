#ifndef XVALIDATE_HPP
#define XVALIDATE_HPP
#include "tools/Logger.hpp"
class IFormatTestSuite;

/**
 * @brief Registers an Test.
 * @param
*/
void RegisterTestSuite(std::shared_ptr<IFormatTestSuite>);
IFormatTestSuite* GetTestSuite(std::string formatName);


Logger& GlobalLog();

#endif //! XVALIDATE_HPP