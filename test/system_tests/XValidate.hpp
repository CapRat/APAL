#ifndef XVALIDATE_HPP
#define XVALIDATE_HPP
#include "tools/Logger.hpp"
#include<memory>
class IFormatTestSuite;

/**
 * @brief Registers an Test.
 * @param
 */
void RegisterTestSuite(std::shared_ptr<IFormatTestSuite>);
IFormatTestSuite*
GetTestSuite(const std::string& formatName);

Logger&
GlobalLog();

#endif //! XVALIDATE_HPP