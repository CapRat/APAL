#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <iostream>

enum class LoggerValue {
    INFO, // Just information, to make it more readable
    STATUS, // Normal statusresult, like from Tests
    WARNING, // Problems in a test, or things, which should be handled.
    ERROR    // Bad thinks, like Failed tests
};

class Logger {
protected:
    int gapIdent = 0;
    std::string getGapAsString() {
        std::string spaceGap;
        for (int i = 0; i < gapIdent; i++) {
            spaceGap += "  ";
        }
        return spaceGap;
    }
public:

    inline void log(const std::string& msg, LoggerValue ) {
        std::cout << msg;
    }
    inline void logN(const std::string& msg, LoggerValue ) {
        std::cout << msg << std::endl;
    }
    inline void logIf(bool cond, const std::string& msg, LoggerValue val) {
        if (cond)
            logN(msg, val);
    }


    inline void rawPrint(const std::string& msg) {
        std::cout << msg;
    }



};
#endif //! LOGGER_HPP