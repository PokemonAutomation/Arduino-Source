/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldTrigger_H
#define PokemonAutomation_PokemonSwSh_OverworldTrigger_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "PokemonSwSh_OverworldTargetTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class OverworldTrigger{
public:
    OverworldTrigger(ProgramEnvironment& env);

    virtual void run(
        InterruptableCommandSession& session,
        OverworldTargetTracker& target_tracker
    ) = 0;

protected:
    static void whistle(const BotBaseContext& context, bool rotate);

protected:
    ProgramEnvironment& m_env;
};



class OverworldTrigger_Whistle : public OverworldTrigger{
public:
    using OverworldTrigger::OverworldTrigger;
    virtual void run(
        InterruptableCommandSession& session,
        OverworldTargetTracker& target_tracker
    ) override;

private:
    bool m_first_after_battle = true;
};


class OverworldTrigger_WhistleStaticAction : public OverworldTrigger{
public:
    OverworldTrigger_WhistleStaticAction(
        ProgramEnvironment& env,
        bool whistle_first,
        size_t whistle_count,
        size_t action_count
    );
    virtual void run(
        InterruptableCommandSession& session,
        OverworldTargetTracker& target_tracker
    ) override;

protected:
    virtual void action(const BotBaseContext& context) = 0;

private:
    bool m_whistle_first;
    size_t m_whistle_count;
    size_t m_action_count;
    bool m_first_after_battle = true;
};


class OverworldTrigger_WhistleCircle : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(const BotBaseContext& context) override;
};


class OverworldTrigger_WhistleHorizontal : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(const BotBaseContext& context) override;
};


class OverworldTrigger_WhistleVertical : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(const BotBaseContext& context) override;
};



}
}
}
#endif
