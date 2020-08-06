#ifndef SPEAKER_HPP
#define SPEAKER_HPP
#include <cstdint>
#include <type_traits>
#define SPOS_TO_SCONF(SpeakerConfigurationtype) static_cast<std::underlying_type<SpeakerConfiguration>::type> (SpeakerConfigurationtype)
namespace XPlug {
    enum class SpeakerConfiguration :uint64_t;

    /**
      * @brief Enum, which represents different Positions of speakers.
      * Channellayout is closed to WAV specification
      */
    enum class SpeakerPosition :uint64_t {
        Undefined = 0,
        FrontLeft = 1 << 0,
        FrontRight = 1 << 1,
        FrontCenter = 1 << 2,
        LowFrequency = 1 << 3,
        RearLeft = 1 << 4,
        RearRight = 1 << 5,
        FrontLeftOfCenter = 1 << 6,
        FrontRightOfCenter = 1 << 7,
        RearCenter = 1 << 8,
        SideLeft = 1 << 9,
        SideRight = 1 << 10,
        TopCenter = 1 << 11,
        FrontLeftHeight = 1 << 12,
        FrontCenterHeight = 1 << 13,
        FrontRightHeight = 1 << 14,
        RearLeftHeight = 1 << 15,
        RearCenterHeight = 1 << 16,
        RearRightHeight = 1 << 17,
        Subbass = 1 << 18
    };


   // constexpr std::underlying_type<SpeakerConfiguration>::type operator &(SpeakerPosition lhs) { return static_cast<std::underlying_type<SpeakerPosition>::type> (lhs); }
    constexpr std::underlying_type<SpeakerConfiguration>::type operator |(std::underlying_type<SpeakerConfiguration>::type lhs, SpeakerPosition rhs) { return static_cast<std::underlying_type<SpeakerConfiguration>::type> (static_cast<std::underlying_type<SpeakerConfiguration>::type>(lhs) | static_cast<std::underlying_type<SpeakerPosition>::type>(rhs)); }
    constexpr std::underlying_type<SpeakerConfiguration>::type operator |(SpeakerConfiguration lhs, SpeakerPosition rhs) { return static_cast<std::underlying_type<SpeakerConfiguration>::type> (static_cast<std::underlying_type<SpeakerConfiguration>::type>(lhs) | static_cast<std::underlying_type<SpeakerPosition>::type>(rhs)); }
    constexpr std::underlying_type<SpeakerConfiguration>::type operator |(SpeakerPosition lhs, SpeakerPosition rhs) { return static_cast<std::underlying_type<SpeakerConfiguration>::type> (static_cast<std::underlying_type<SpeakerPosition>::type>(lhs) | static_cast<std::underlying_type<SpeakerPosition>::type>(rhs)); }

    enum class SpeakerConfiguration :uint64_t {
        Undefined = SPOS_TO_SCONF(SpeakerPosition::Undefined),
        Mono = SPOS_TO_SCONF(SpeakerPosition::FrontCenter), // Mono sound
        MonoLegacy = SPOS_TO_SCONF(SpeakerPosition::FrontLeft), // Mono implementations for old speakers.
        Stereo2_0 = SpeakerPosition::FrontLeft | SpeakerPosition::FrontRight,
        Stereo2_1 = Stereo2_0 | SpeakerPosition::LowFrequency,
        Stereo3_0 = Stereo2_0 | SpeakerPosition::FrontCenter,
        Stereo3_1 = Stereo3_0 | SpeakerPosition::LowFrequency,
        Surround3_0 = Stereo2_0 | SpeakerPosition::RearCenter,
        Surround4_0 = Surround3_0 | SpeakerPosition::FrontCenter,
        Surround4_1 = Surround4_0 | SpeakerPosition::LowFrequency,
        Surround5_0Front = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::RearLeft | SpeakerPosition::RearRight,
        Surround5_1Front = Surround5_0Front | SpeakerPosition::LowFrequency,
        Surround5_0Side = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::SideLeft | SpeakerPosition::SideRight,
        Surround5_1Side = Surround5_0Side | SpeakerPosition::LowFrequency,
        Atmos5_1_4 = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::FrontLeftHeight | SpeakerPosition::FrontRightHeight | SpeakerPosition::LowFrequency,
        Surround6_0Back = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::RearLeft | SpeakerPosition::RearRight | SpeakerPosition::RearCenter,
        Surround6_1Back = Surround6_0Back | SpeakerPosition::LowFrequency,
        Surround6_0Front = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::RearLeft | SpeakerPosition::FrontLeftOfCenter | SpeakerPosition::FrontRightOfCenter,
        Surround6_1Front = Surround6_0Front | SpeakerPosition::LowFrequency,
        Surround6_0Side = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::RearLeft | SpeakerPosition::SideLeft | SpeakerPosition::SideRight,
        Surround6_1Side = Surround6_0Side | SpeakerPosition::LowFrequency,
        Wide7_1 = Stereo2_0 | SpeakerPosition::FrontCenter | SpeakerPosition::FrontLeftOfCenter | SpeakerPosition::FrontRightOfCenter | SpeakerPosition::RearLeft | SpeakerPosition::RearRight | SpeakerPosition::LowFrequency,







        /**SURROUND ALIASSE */
        Surround5_0 = Surround5_0Side,
        Surround5_1 = Surround5_1Side,

    };

    /**
     * @brief  Gets the SpeakerPosition from given index. Stereo for example will return with index 0 SpeakerPosition::Left and with index 1 SpeakerPosition::Right.
     * The Templateparameter is the SpeakerConfiguration to analyze
     * @param index Index of the SpeakerPosition to get.
     * @return single Bitmask with only 1 bit set. This can be represented as SpeakerPosition.
     */
    template<SpeakerConfiguration c>
    constexpr SpeakerPosition getSpeakerPositionAt(size_t index) {
        size_t indexCounter = 0;
        for (size_t i = 0; i < sizeof(c); i++) {
            if (static_cast<uint64_t>(c) & ((uint64_t)1 << i)) {//check if bit is set in pos
                if (indexCounter == index)
                    return static_cast<SpeakerPosition>(1 << i);
                indexCounter++;
            }
        }
        return SpeakerPosition::Undefined;
    }

    template<SpeakerConfiguration c>
    constexpr const char* getSpeakerSuffix(size_t index) {
        switch (getSpeakerPositionAt<c>(index)) {
        case SpeakerPosition::FrontCenter:
            return "FC";
        case SpeakerPosition::FrontLeft:
            return "FL";
        case SpeakerPosition::FrontRight:
            return "FR";
        case SpeakerPosition::FrontLeftHeight:
            return "FLH";
        case SpeakerPosition::FrontRightHeight:
            return "FRH";
        case SpeakerPosition::FrontLeftOfCenter:
            return "FLC";
        case SpeakerPosition::FrontRightOfCenter:
            return "FRC";
        default:
            return "";
        }
    }
    template<>
    constexpr const char* getSpeakerSuffix<SpeakerConfiguration::Mono>(size_t) {
        return "";
    }
    template<>
    constexpr const char* getSpeakerSuffix<SpeakerConfiguration::MonoLegacy>(size_t) {
        return "";
    }
}
#endif //! SPEAKER_HPP