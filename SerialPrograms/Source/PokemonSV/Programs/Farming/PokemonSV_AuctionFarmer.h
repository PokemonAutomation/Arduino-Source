/*  Auction Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AuctionFarmer_H
#define PokemonAutomation_PokemonSV_AuctionFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "PokemonSV/Options/PokemonSV_AuctionItemTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

struct AuctionOffer{
    std::string item;
};

class AuctionFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AuctionFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class AuctionFarmer : public SingleSwitchProgramInstance{
public:
    AuctionFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    AuctionItemTable TARGET_ITEMS;
    
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_AUCTION_WIN;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption ONE_NPC;

    std::vector<ImagePixelBox> detect_dialog_boxes(const ImageViewRGB32& screen);
    void reset_auctions(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool do_full_reset, uint8_t& year);
    std::vector<std::pair<AuctionOffer, ImageFloatBox>> check_offers(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void move_to_auctioneer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer wanted);
    void move_dialog_to_center(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer offer);
    void bid_on_item(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer offer);
    bool is_good_offer(AuctionOffer);
    void reset_position(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
};




}
}
}
#endif
