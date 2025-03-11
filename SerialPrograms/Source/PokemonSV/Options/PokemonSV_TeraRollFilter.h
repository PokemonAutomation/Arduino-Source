/*  Tera Roll Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRollFilter_H
#define PokemonAutomation_PokemonSV_TeraRollFilter_H

#include <set>
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

class TeraCardReader;


struct TeraRaidData{

#if 0
    enum class EventType{
        UNKNOWN,
        NORMAL,
        EVENT,
    };
    EventType event_type = EventType::UNKNOWN;
#endif

    uint8_t stars = 0;
    std::string tera_type;
    std::set<std::string> species;
};



class TeraRollFilter : public GroupOption{
public:
    TeraRollFilter(uint8_t default_max_stars, bool enable_herb_filter);

    virtual std::string check_validity() const override;

    enum class FilterResult{
        NO_RAID,
        FAILED,
        PASSED,
    };

    FilterResult run_filter(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        TeraRaidData& data
    ) const;


private:
    void read_card(
        const ProgramInfo& info, VideoStream& stream, const ImageViewRGB32& screen,
        TeraCardReader& reader, TeraRaidData& data
    ) const;
    bool check_herba(const std::set<std::string>& pokemon_slugs) const;

public:
    enum class EventCheckMode{
        CHECK_ALL,
        CHECK_ONLY_EVENT,
        CHECK_ONLY_NONEVENT,
    };
    EnumDropdownOption<EventCheckMode> EVENT_CHECK_MODE;

    SimpleIntegerOption<uint8_t> MIN_STARS;
    SimpleIntegerOption<uint8_t> MAX_STARS;

    BooleanCheckBoxOption SKIP_NON_HERBA;
};



}
}
}
#endif
