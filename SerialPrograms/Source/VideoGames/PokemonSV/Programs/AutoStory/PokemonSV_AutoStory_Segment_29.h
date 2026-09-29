/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_29_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_29_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_29 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

class AutoStory_Checkpoint_71 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_72 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_73 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_74 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};



// start: At Montenevera Pokecenter
// end: Spoke to Montenevera gym receptionist. Inside Montenevera gym building.
void checkpoint_71(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, EventNotificationOption& notif_error_recoverable, AutoStoryStats& stats, const std::string& checkpoint_text);

// start: Spoke to Montenevera gym receptionist. Inside Montenevera gym building.
// end: Passed gym test with MC Sledge. Standing in front of MC Sledge, outside the Montenevera gym building.
void checkpoint_72(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, EventNotificationOption& notif_error_recoverable, AutoStoryStats& stats, const std::string& checkpoint_text);

// start: Passed gym test with MC Sledge. Standing in front of MC Sledge, outside the Montenevera gym building.
// end: Beat Montenevera Gym. Inside Montenevera gym building.
void checkpoint_73(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, EventNotificationOption& notif_error_recoverable, AutoStoryStats& stats, const std::string& checkpoint_text);

// start: Beat Montenevera Gym. Inside Montenevera gym building.
// end: At Glaseado gym Pokecenter.
void checkpoint_74(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, EventNotificationOption& notif_error_recoverable, AutoStoryStats& stats, const std::string& checkpoint_text);

// moves player from Montenevera Pokecenter to Glaseado Gym Pokecenter
void move_from_montenevera_to_glaseado_gym(SingleSwitchProgramEnvironment& env, ProControllerContext& context);




}
}
}
#endif
