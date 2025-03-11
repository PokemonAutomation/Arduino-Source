/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldTrigger_H
#define PokemonAutomation_PokemonSwSh_OverworldTrigger_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh_OverworldTargetTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class OverworldTrigger{
public:
    OverworldTrigger(OverworldTargetTracker& target_tracker);
    virtual ~OverworldTrigger() = default;

    virtual void run(ProControllerContext& context) = 0;

protected:
    void whistle(ProControllerContext& context, bool rotate);

protected:
    OverworldTargetTracker& m_target_tracker;
};



class OverworldTrigger_Whistle : public OverworldTrigger{
public:
    using OverworldTrigger::OverworldTrigger;
    virtual void run(ProControllerContext& context) override;

private:
    bool m_first_after_battle = true;
};


class OverworldTrigger_WhistleStaticAction : public OverworldTrigger{
public:
    OverworldTrigger_WhistleStaticAction(
        OverworldTargetTracker& target_tracker,
        bool whistle_first,
        size_t whistle_count,
        size_t action_count
    );
    virtual void run(ProControllerContext& context) override;

protected:
    virtual void action(ProControllerContext& context) = 0;

private:
    void whistle_loop(ProControllerContext& context);
    void action_loop(ProControllerContext& context);

private:
    bool m_whistle_first;
    size_t m_whistle_count;
    size_t m_action_count;
    bool m_first_after_battle = true;
};


class OverworldTrigger_WhistleCircle : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(ProControllerContext& context) override;
};


class OverworldTrigger_WhistleHorizontal : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(ProControllerContext& context) override;
};


class OverworldTrigger_WhistleVertical : public OverworldTrigger_WhistleStaticAction{
public:
    using OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction;
    virtual void action(ProControllerContext& context) override;
};



}
}
}
#endif
