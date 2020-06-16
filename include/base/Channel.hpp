#ifndef CHANNEL_HPPP
#define CHANNEL_HPP
#include <interfaces/IPort.hpp>
namespace XPlug {
 

    /**
     * @brief  Gets the SpeakerPosition from given index. Stereo for example will return with index 0 SpeakerPosition::Left and with index 2 SpeakerPosition::Right.
     * The Templateparameter is the SpeakerConfiguration to analyze
     * @param index Index of the SpeakerPosition to get.
     * @return single Bitmask with only 1 bit set. This can be represented as SpeakerPosition.
     */
    template<SpeakerConfiguration c>
    SpeakerPosition getSpeakerPositionAt(size_t index) {
        size_t indexCounter = 0;
        for (int i = 1; i < sizeof(c); i++) {
            if (static_cast<uint64_t>(c) & ((uint64_t) 1 << i)) {//check if bit is set in pos
                if (indexCounter == index)
                    return static_cast<SpeakerPosition>(1 << i);
                indexCounter++;
            }
        }
        return SpeakerPosition::Undefined;
    }

    /**
     * @brief Templatespecialication for Mono.
     * @param index Index of the SpeakerPosition to get.
     * @return SpekaerPostion::Center. Everytime. No indexchecking.
    */
   /* template<>
    SpeakerPosition getSpeakerPositionAt <SpeakerConfiguration::Mono> (size_t index) {
        return SpeakerPosition::Center;
    }*/

    
}
#endif //!