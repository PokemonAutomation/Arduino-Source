/*  Auction Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_AuctionItemNameReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Resources/PokemonSV_AuctionItemNames.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "PokemonSV_AuctionFarmer.h"

#include <iostream>
#include <unordered_map>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AuctionFarmer_Descriptor::AuctionFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AuctionFarmer",
        STRING_POKEMON + " SV", "Auction Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AuctionFarmer.md",
        "Check auctions and bid on items.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct AuctionFarmer_Descriptor::Stats : public StatsTracker {
    Stats()
        : m_resets(m_stats["Resets"])
        , m_auctions(m_stats["Auctions"])
        , m_money(m_stats["Spent Money"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Auctions");
        m_display_order.emplace_back("Spent Money");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_resets;
    std::atomic<uint64_t>& m_auctions;
    std::atomic<uint64_t>& m_money;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> AuctionFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AuctionFarmer::AuctionFarmer()
    : LANGUAGE(
        "<b>Game Language:</b><br>The language is needed to read which items are offered.",
        AuctionItemNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TARGET_ITEMS("<b>Items:</b><br>Multiple Items can be selected. The program will bid on any selected item which is offered.")
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_AUCTION_WIN("Auction Win", true, false, ImageAttachmentMode::JPG, {"Notifs"})
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_AUCTION_WIN,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options("<font size=4><b>Advanced Options: (developer only)</b></font>")
    , ONE_NPC("<b>One NPC:</b><br>Check only the NPC you're standing in front of. (Multiple NPCs in development)", LockMode::LOCK_WHILE_RUNNING, true)
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ITEMS);
    PA_ADD_OPTION(NOTIFICATIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE) {
        PA_ADD_STATIC(m_advanced_options);
        PA_ADD_OPTION(ONE_NPC);
    }
}


std::vector<ImageFloatBox> AuctionFarmer::detect_dialog_boxes(const ImageViewRGB32& screen) {
    using namespace Kernels::Waterfill;

    uint32_t MIN_BORDER_THRESHOLD = 0xffc07000;
    uint32_t MAX_BORDER_THRESHOLD = 0xffffc550;

    uint32_t MIN_YELLOW_THRESHOLD = 0xffd0b000;
    uint32_t MAX_YELLOW_THRESHOLD = 0xffffff30;

    size_t width = screen.width();
    size_t height = screen.height();


    std::vector<ImageFloatBox> dialog_boxes;
    {
        PackedBinaryMatrix border_matrix = compress_rgb32_to_binary_range(screen, MIN_BORDER_THRESHOLD, MAX_BORDER_THRESHOLD);

        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(border_matrix);
        auto iter = session->make_iterator(50);
        WaterfillObject object;
        while (iter->find_next(object, true)) {
            //  Discard objects touching the edge of the screen or bottom right corner where the map is located or if the object is too small
            if (object.min_x == 0 || object.min_y == 0 // touching left or top edge
                || object.max_x + 1 >= width || object.max_y + 1 >= height // touching right or bottom edge
                || (object.max_x > width * 0.82 && object.max_y > height * 0.68) // touches mini map area
                || object.width() < width * 0.0926 || object.height() < height * 0.0926 // object is too small
            ){
                continue;
            }

            // check for yellow inside the orange border
            ImagePixelBox border_pixel_box(object);
            ImageFloatBox border_float_box = pixelbox_to_floatbox(screen, border_pixel_box);
            ImageViewRGB32 dialog = extract_box_reference(screen, border_pixel_box);

            PackedBinaryMatrix yellow_matrix = compress_rgb32_to_binary_range(dialog, MIN_YELLOW_THRESHOLD, MAX_YELLOW_THRESHOLD);

            std::unique_ptr<WaterfillSession> yellow_session = make_WaterfillSession(yellow_matrix);
            auto yellow_iter = yellow_session->make_iterator(300);
            WaterfillObject yellow_object;
            while (yellow_iter->find_next(yellow_object, true)) {
                //  Discard small objects
                if (object.width() < width * 0.0925 || object.height() < height * 0.0925) {
                    continue;
                }
                ImagePixelBox dialog_pixel_box(yellow_object);
                
                ImageFloatBox translated_dialog_box = translate_to_parent(screen, border_float_box, dialog_pixel_box);
                dialog_boxes.emplace_back(translated_dialog_box);
            }
        }
    }
    return dialog_boxes;
}


void AuctionFarmer::reset_auctions(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool do_full_reset, uint8_t& year){
    try{
        if (do_full_reset){
            if (year == MAX_YEAR){
                pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
                home_roll_date_enter_game_autorollback(env.console, context, year);
            }
            save_game_from_overworld(env.program_info(), env.console, context);

            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            home_roll_date_enter_game_autorollback(env.console, context, year);
        }
        pbf_wait(context, 1 * TICKS_PER_SECOND);

        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        reset_game_from_home(env.program_info(), env.console, context, TICKS_PER_SECOND);
    }catch (OperationFailedException& e){
        AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();
        stats.m_errors++;
        env.update_stats();
        throw FatalProgramException(std::move(e));
    }
}

std::vector<std::pair<AuctionOffer, ImageFloatBox>> AuctionFarmer::check_offers(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    pbf_wait(context, 2 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    
    VideoSnapshot screen = env.console.video().snapshot();
    std::vector<ImageFloatBox> dialog_boxes = detect_dialog_boxes(screen);
    std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers;

    if (dialog_boxes.empty()) {
        stats.m_errors++;
        send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, "Could not detect any offer dialogs.", screen);
    }

    // read dialog bubble
    for (ImageFloatBox dialog_box : dialog_boxes){
        OverlayBoxScope dialog_overlay(env.console, dialog_box, COLOR_DARK_BLUE);

        ImageFloatBox offer_box(0.05, 0.02, 0.90, 0.49);
        ImageFloatBox translated_offer_box = translate_to_parent(screen, dialog_box, floatbox_to_pixelbox(dialog_box.width, dialog_box.height, offer_box));
        OverlayBoxScope offer_overlay(env.console, translated_offer_box, COLOR_BLUE);
        
        ImageViewRGB32 dialog = extract_box_reference(screen, dialog_box);
        ImageViewRGB32 offer_image = extract_box_reference(dialog, offer_box);


        const double LOG10P_THRESHOLD = -1.5;
        std::string best_item;
        OCR::StringMatchResult result = AuctionItemNameReader::instance().read_substring(
            env.console, LANGUAGE,
            offer_image,
            OCR::BLACK_TEXT_FILTERS()
        );

        result.clear_beyond_log10p(LOG10P_THRESHOLD);
        if (best_item.empty() && !result.results.empty()) {
            auto iter = result.results.begin();
            if (iter->first < LOG10P_THRESHOLD) {
                best_item = iter->second.token;

                AuctionOffer offer{ best_item };
                std::pair<AuctionOffer, ImageFloatBox> pair(offer, dialog_box);
                offers.emplace_back(pair);
            }
        }
    }
    return offers;
}

bool AuctionFarmer::is_good_offer(AuctionOffer offer) {
    // Special handling for Japanese bottle cap items
    bool any_bottle_cap = false;
    if (LANGUAGE == Language::Japanese) {
        any_bottle_cap = (offer.item == "bottle-cap" || offer.item == "gold-bottle-cap") 
            && (TARGET_ITEMS.find_item("bottle-cap") || TARGET_ITEMS.find_item("gold-bottle-cap"));
    }

    return TARGET_ITEMS.find_item(offer.item) || any_bottle_cap ;
}

// Move to auctioneer and interact
void AuctionFarmer::move_to_auctioneer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer offer) {
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);

    size_t tries = 0;
    while (tries < 10) {
        if (!ONE_NPC) {
            move_dialog_to_center(env, context, offer);
            pbf_move_left_joystick(context, 128, 0, 60, 10);
        }

        pbf_press_button(context, BUTTON_A, 20, 100);
        int ret = wait_until(env.console, context, Milliseconds(4000), { advance_detector });

        if (ret == 0) {
            return;
        }
        tries++;
    }
    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT, env.console,
        "Too many attempts to talk to the NPC.",
        true
    );
}

// Dialog is the only piece of orientation we have, so the goal is to put it into the center of the screen so we know in which direction the character walks.
// This is only used for multiple NPCs.
void AuctionFarmer::move_dialog_to_center(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer wanted) {
    float center_x = 0.0f;
    float center_y = 0.0f;
    bool offer_visible = false;

    while (center_x < 0.43 || center_x > 0.57) {
        context.wait_for_all_requests();
        std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers = check_offers(env, context);

        for (std::pair<AuctionOffer, ImageFloatBox> offer : offers) {
            if (offer.first.item != wanted.item) {
                continue;
            }
            offer_visible = true;

            center_x = offer.second.x + (0.5 * offer.second.width);
            center_y = offer.second.y + (0.5 * offer.second.height);


            // check whether the stop condition is fullfilled by now.
            if (!(center_x < 0.43 || center_x > 0.57)) {
                break;
            }

            uint8_t distance_x = (uint16_t)((center_x) * 255);
            uint8_t distance_y = (uint16_t)((center_y * 255));
            env.console.log(std::to_string(distance_x));
            env.console.log(std::to_string(distance_y));

            pbf_move_right_joystick(context, distance_x, distance_y, 20, 20);

            break;
        }

        if (!offer_visible) {
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Lost offer dialog for wanted item.",
                true
            );
        }
    }
}

void AuctionFarmer::reset_position(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    if (ONE_NPC) {
        // No movement, player character should always be directly in front of an auctioneer.
        return;
    }

    // move backwards, TODO: check position(?) and orientation
    pbf_move_left_joystick(context, 128, 255, 50, 20);
    return;
}


uint64_t read_next_bid(ConsoleHandle& console, BotBaseContext& context, bool high) {
    float box_y = high ? 0.42f : 0.493f;
    OverlayBoxScope box(console, { 0.73, box_y, 0.17, 0.048 });
    std::unordered_map<uint64_t, size_t> read_bids;
    size_t highest_read = 0;
    uint64_t read_value = 0;

    // read next bid multiple times since the selection arrow sometimes blocks the first digit
    for (size_t i = 0; i < 10; i++) {
        VideoSnapshot screen = console.video().snapshot();
        uint64_t read_bid = OCR::read_number(console.logger(), extract_box_reference(screen, box));

        if (read_bids.find(read_bid) == read_bids.end()) {
            read_bids[read_bid] = 0;
        }
        read_bids[read_bid] += 1;

        if (read_bids[read_bid] > highest_read) {
            highest_read = read_bids[read_bid];
            read_value = read_bid;
        }
        context.wait_for(Milliseconds(20));
    }

    console.log("Next bid: " + std::to_string(read_value));
    return read_value;
}

void AuctionFarmer::bid_on_item(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AuctionOffer offer) {
    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    VideoSnapshot offer_screen = env.console.video().snapshot();

    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    PromptDialogWatcher high_detector(COLOR_RED, { 0.50, 0.40, 0.40, 0.082 });
    PromptDialogWatcher mid_detector(COLOR_PURPLE, { 0.50, 0.475, 0.40, 0.082 });
    PromptDialogWatcher low_detector(COLOR_PURPLE, { 0.50, 0.55, 0.40, 0.082 });
    OverworldWatcher overworld_detector(COLOR_BLUE);
    bool won_auction = true;
    bool auction_ongoing = true;
    int64_t current_bid = 0;

    context.wait_for_all_requests();
    while (auction_ongoing) {
        int ret = wait_until(env.console, context, Milliseconds(5000), { advance_detector, high_detector, mid_detector, low_detector, overworld_detector });
        context.wait_for(Milliseconds(100));

        switch (ret) {
        case 0:
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 1:
            current_bid = read_next_bid(env.console, context, true);
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 2:
            current_bid = read_next_bid(env.console, context, false);
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 3:
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 4:
            auction_ongoing = false;
            break;
        default:
            break;
        }
        context.wait_for_all_requests();
    }

    if (won_auction) {
        stats.m_auctions++;
        if (current_bid >= 0) {
            stats.m_money += current_bid;
        }
        env.update_stats();
        send_program_notification(
            env, NOTIFICATION_AUCTION_WIN,
            COLOR_GREEN, "Auction won!",
            {
                { "Item:", get_auction_item_name(offer.item).display_name() },
                { "Final Bid:", std::to_string(current_bid) },
            }
            , "", offer_screen);
    }

    return;
}

void AuctionFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);
    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    uint8_t year = MAX_YEAR;


    while (true) {
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        reset_auctions(env, context, true, year);
        stats.m_resets++;
        env.update_stats();
        
        bool good_offer = false;
        while (!good_offer) {
            size_t npc_tries = 0;
            if (!ONE_NPC) {
                pbf_move_right_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
            }

            std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers = check_offers(env, context);
            for (std::pair<AuctionOffer, ImageFloatBox>& offer_pair : offers){
                AuctionOffer offer = offer_pair.first;
                if (is_good_offer(offer)) {
                    try {
                        move_to_auctioneer(env, context, offer);
                    }
                    catch (OperationFailedException& e){
                        stats.m_errors++;
                        e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

                        npc_tries++;
                        // if ONE_NPC the program already tries multiple times without change to compensate for dropped inputs
                        // at this point it is more likely to be non-recoverable
                        size_t max_npc_tries = ONE_NPC ? 1 : 3;

                        if (npc_tries < max_npc_tries) {
                            VideoSnapshot screen = env.console.video().snapshot();
                            send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, e.message(), screen);
                        }
                        else {
                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT, env.console,
                                "Failed to talk to the NPC!",
                                true
                            );
                        }
                        break;
                    }

                    bid_on_item(env, context, offer);
                    reset_position(env, context);

                    good_offer = true;
                }
            }
            if (!good_offer){
                reset_auctions(env, context, false, year);
                stats.m_resets++;
            }

            env.update_stats();
            pbf_wait(context, 125);
            context.wait_for_all_requests();
        }
    }
        
}




}
}
}
