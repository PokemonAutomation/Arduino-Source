#ifndef ENRICHMENT_H
#define ENRICHMENT_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class Enrichment_Descriptor : public SingleSwitchProgramDescriptor{
public:
    Enrichment_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

struct Game{
    std::string name;
    int index;
    bool accessed;
    bool depleted;
};

class Enrichment : public SingleSwitchProgramInstance{
public:
    Enrichment();

    void home_put_away_pokemon(SingleSwitchProgramEnvironment&, ProControllerContext&, Game&, bool);
    void home_dispose_of_go(SingleSwitchProgramEnvironment&, ProControllerContext&);
    bool initialize_home(SingleSwitchProgramEnvironment&, ProControllerContext&);
    void block1(SingleSwitchProgramEnvironment&, ProControllerContext&, std::vector<Game>&);
    void block2(SingleSwitchProgramEnvironment&, ProControllerContext&, std::vector<bool>&, bool&, bool&);
    void block3(SingleSwitchProgramEnvironment&, ProControllerContext&, std::vector<Game>&);
    virtual void program(SingleSwitchProgramEnvironment&, ProControllerContext&) override;

private:
    SimpleIntegerOption<uint16_t> HOME_FIRST_BOX;
    SimpleIntegerOption<uint16_t> HOME_LAST_BOX;
    SimpleIntegerOption<uint16_t> PLA_FIRST_BOX;
    SimpleIntegerOption<uint16_t> PLA_LAST_BOX;
    StringOption SV_BOX_NAME;
    BooleanCheckBoxOption WIPE_MARKINGS;
    BooleanCheckBoxOption DISPOSE_GOS;
    BooleanCheckBoxOption STARTING_AT_DESK;
    BooleanCheckBoxOption EMERGENCY_DELOAD;
    BooleanCheckBoxOption NORMAL_DELOAD;
    BooleanCheckBoxOption SKIP_SETUP;
    EventNotificationsOption NOTIFICATIONS;
};
}
}
}

#endif // POKEMONHOME_ENRICHMENT_H
