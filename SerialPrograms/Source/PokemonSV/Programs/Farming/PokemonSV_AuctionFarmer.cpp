/*  Auction Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include <iostream>
#include <unordered_map>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_AuctionItemNameReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Resources/PokemonSV_AuctionItemNames.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV_AuctionFarmer.h"


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
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct AuctionFarmer_Descriptor::Stats : public StatsTracker{
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
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_STATIC(m_advanced_options);
        PA_ADD_OPTION(ONE_NPC);
    }
}


std::vector<ImagePixelBox> AuctionFarmer::detect_dialog_boxes(const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    uint32_t MIN_BORDER_THRESHOLD = 0xffc07000;
    uint32_t MAX_BORDER_THRESHOLD = 0xffffc550;

    uint32_t MIN_YELLOW_THRESHOLD = 0xffd0b000;
    uint32_t MAX_YELLOW_THRESHOLD = 0xffffff30;

    size_t width = screen.width();
    size_t height = screen.height();


    std::vector<ImagePixelBox> dialog_boxes;
    {
        PackedBinaryMatrix border_matrix = compress_rgb32_to_binary_range(screen, MIN_BORDER_THRESHOLD, MAX_BORDER_THRESHOLD);

        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(border_matrix);
        auto iter = session->make_iterator(50);
        WaterfillObject object;
        while (iter->find_next(object, true)){
            //  Discard objects touching the edge of the screen or bottom right corner where the map is located or if the object is too small
            if (object.min_x == 0 || object.min_y == 0 // touching left or top edge
                || object.max_x + 1 >= width || object.max_y + 1 >= height // touching right or bottom edge
                || (object.max_x > width * 0.82 && object.max_y > height * 0.68) // touches mini map area
                || object.width() < width * 0.0926 || object.height() < height * 0.0926 // object is too small
            ){
                continue;
            }
            dialog_boxes.emplace_back(object);

//            static int c = 0;
//            extract_box_reference(screen, object).save("image-" + std::to_string(c++) + ".png");

#if 1
            // check for yellow inside the orange border
            ImagePixelBox border_pixel_box(object);
//            ImageFloatBox border_float_box = pixelbox_to_floatbox(screen, border_pixel_box);
            ImageViewRGB32 dialog = extract_box_reference(screen, border_pixel_box);
            PackedBinaryMatrix yellow_matrix = compress_rgb32_to_binary_range(dialog, MIN_YELLOW_THRESHOLD, MAX_YELLOW_THRESHOLD);

            std::unique_ptr<WaterfillSession> yellow_session = make_WaterfillSession(yellow_matrix);
            auto yellow_iter = yellow_session->make_iterator(300);
            WaterfillObject yellow_object;
            while (yellow_iter->find_next(yellow_object, true)){
                //  Discard small objects
                if (object.width() < width * 0.0925 || object.height() < height * 0.0925){
                    continue;
                }

//                extract_box_reference(dialog, yellow_object).save("yellow-" + std::to_string(c++) + ".png");

//                ImagePixelBox dialog_pixel_box(yellow_object);
                ImagePixelBox dialog_pixel_box;
                dialog_pixel_box.min_x = border_pixel_box.min_x + yellow_object.min_x;
                dialog_pixel_box.min_y = border_pixel_box.min_y + yellow_object.min_y;
                dialog_pixel_box.max_x = dialog_pixel_box.min_x + yellow_object.width();
                dialog_pixel_box.max_y = dialog_pixel_box.min_y + yellow_object.height();
                
//                ImageFloatBox translated_dialog_box = translate_to_parent(screen, border_float_box, dialog_pixel_box);
//                dialog_boxes.emplace_back(translated_dialog_box);
                dialog_boxes.emplace_back(dialog_pixel_box);
            }
#endif
        }
    }
    return dialog_boxes;
}


void AuctionFarmer::reset_auctions(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool do_full_reset, uint8_t& year){
    try{
        if (do_full_reset){
            if (year == MAX_YEAR){
                go_home(env.console, context);
                PokemonSwSh::home_roll_date_enter_game_autorollback(env.console, context, year);
            }
            save_game_from_overworld(env.program_info(), env.console, context);

            go_home(env.console, context);
            PokemonSwSh::home_roll_date_enter_game_autorollback(env.console, context, year);
        }
        pbf_wait(context, 1 * TICKS_PER_SECOND);

        go_home(env.console, context);
        context.wait_for_all_requests();
        reset_game_from_home(env.program_info(), env.console, context, TICKS_PER_SECOND);
    }catch (OperationFailedException& e){
        AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();
        stats.m_errors++;
        env.update_stats();
        throw FatalProgramException(std::move(e));
    }
}

std::vector<std::pair<AuctionOffer, ImageFloatBox>> AuctionFarmer::check_offers(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    pbf_wait(context, 2 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    
    VideoSnapshot screen = env.console.video().snapshot();
    std::vector<ImagePixelBox> dialog_boxes = detect_dialog_boxes(screen);
    std::deque<OverlayBoxScope> bubbles_boxes;
    std::deque<OverlayBoxScope> offer_overlay_boxes;
    std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers;

    if (dialog_boxes.empty()){
        stats.m_errors++;
        send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, "Could not detect any offer dialogs.", screen);
    }

    // read dialog bubble
    ImageFloatBox top_offer_box(0.05, 0.02, 0.90, 0.49);
    ImageFloatBox bottom_offer_box(0.05, 0.49, 0.90, 0.49);
    std::vector<ImageFloatBox> offer_boxes = {top_offer_box};
    if (LANGUAGE == Language::Spanish || LANGUAGE == Language::ChineseTraditional) {
        offer_boxes.emplace_back(bottom_offer_box);
    }

    for (ImagePixelBox dialog_box : dialog_boxes){
        for (ImageFloatBox offer_box : offer_boxes) {
            //        std::cout << "dialog_box: ["
            //                << dialog_box.min_x << "," << dialog_box.min_y << "] - ["
            //                << dialog_box.max_x << "," << dialog_box.max_y << "]" << std::endl;

            ImageFloatBox dialog_float_box = pixelbox_to_floatbox(screen, dialog_box);
            bubbles_boxes.emplace_back(env.console, dialog_float_box, COLOR_GREEN);


            //        OverlayBoxScope dialog_overlay(env.console, dialog_box, COLOR_DARK_BLUE);
            ImageFloatBox translated_offer_box = translate_to_parent(
                screen,
                dialog_float_box,
                floatbox_to_pixelbox(dialog_box.width(), dialog_box.height(), offer_box)
            );
            //        std::cout << "translated_offer_box: ["
            //                << translated_offer_box.x << "," << translated_offer_box.y << "] - ["
            //                << translated_offer_box.width << "," << translated_offer_box.height << "]" << std::endl;

            offer_overlay_boxes.emplace_back(env.console, translated_offer_box, COLOR_BLUE);

            //        OverlayBoxScope offer_overlay(env.console, translated_offer_box, COLOR_BLUE);

            ImageViewRGB32 dialog = extract_box_reference(screen, dialog_box);
            ImageViewRGB32 offer_image = extract_box_reference(dialog, offer_box);

            //        std::cout << offer_image.width() << " x " << offer_image.height() << std::endl;


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
                    std::pair<AuctionOffer, ImageFloatBox> pair(offer, dialog_float_box);
                    offers.emplace_back(pair);
                }
            }
        }
    }
//  context.wait_for(std::chrono::seconds(100));
    return offers;
}

bool AuctionFarmer::is_good_offer(AuctionOffer offer){
    // Special handling for Japanese bottle cap items
    bool any_bottle_cap = false;
    if (LANGUAGE == Language::Japanese){
        any_bottle_cap = (offer.item == "bottle-cap" || offer.item == "gold-bottle-cap") 
            && (TARGET_ITEMS.find_item("bottle-cap") || TARGET_ITEMS.find_item("gold-bottle-cap"));
    }

    return TARGET_ITEMS.find_item(offer.item) || any_bottle_cap ;
}

// Move to auctioneer and interact
void AuctionFarmer::move_to_auctioneer(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AuctionOffer offer){
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);

    size_t tries = 0;
    while (tries < 10){
        if (!ONE_NPC){
            move_dialog_to_center(env, context, offer);
            pbf_move_left_joystick(context, 128, 0, 60, 10);
        }

        pbf_press_button(context, BUTTON_A, 20, 100);
        int ret = wait_until(env.console, context, Milliseconds(4000), { advance_detector });

        if (ret == 0){
            return;
        }
        tries++;
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Too many attempts to talk to the NPC.",
        env.console
    );
}

// Dialog is the only piece of orientation we have, so the goal is to put it into the center of the screen so we know in which direction the character walks.
// This is only used for multiple NPCs.
void AuctionFarmer::move_dialog_to_center(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AuctionOffer wanted){
    double center_x = 0.0f;
    double center_y = 0.0f;
    bool offer_visible = false;

    while (center_x < 0.43 || center_x > 0.57){
        context.wait_for_all_requests();
        std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers = check_offers(env, context);

        for (std::pair<AuctionOffer, ImageFloatBox> offer : offers){
            if (offer.first.item != wanted.item){
                continue;
            }
            offer_visible = true;

            center_x = offer.second.x + (0.5 * offer.second.width);
            center_y = offer.second.y + (0.5 * offer.second.height);


            // check whether the stop condition is fulfilled by now.
            if (!(center_x < 0.43 || center_x > 0.57)){
                break;
            }

            uint8_t distance_x = (uint8_t)(center_x * 255);
            uint8_t distance_y = (uint8_t)(center_y * 255);
            env.console.log(std::to_string(distance_x));
            env.console.log(std::to_string(distance_y));

            pbf_move_right_joystick(context, distance_x, distance_y, 20, 20);

            break;
        }

        if (!offer_visible){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Lost offer dialog for wanted item.",
                env.console
            );
        }
    }
}

void AuctionFarmer::reset_position(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (ONE_NPC){
        // No movement, player character should always be directly in front of an auctioneer.
        return;
    }

    // move backwards, TODO: check position(?) and orientation
    pbf_move_left_joystick(context, 128, 255, 50, 20);
    return;
}


uint64_t read_next_bid(VideoStream& stream, ProControllerContext& context, Language language, bool high){
    // How much to cut off from the bid text in order to not read currencies and exclamation marks as numbers.
    // Values are pixels for a 1920x1080 screen, negative values are padding
    static const std::map<Language, std::pair<int, int>> cutoffs = {
        { Language::English, {22, 6} },
        { Language::Japanese, {-5, 54} },
        { Language::Spanish, {6, 32} },
        { Language::French, {-5, 45} },
        { Language::German, {-5, 22} },
        { Language::Italian, {-5, 35} },
        { Language::Korean, {-5, 42} },
        { Language::ChineseSimplified, {22, 7} },
        { Language::ChineseTraditional, {22,7} }
    };

    static const std::map<Language, float> high_x = {
        { Language::English, 0.75f },
        { Language::Japanese, 0.75f },
        { Language::Spanish, 0.73f },
        { Language::French, 0.68f },
        { Language::German, 0.73f },
        { Language::Italian, 0.75f },
        { Language::Korean, 0.75f },
        { Language::ChineseSimplified, 0.75f },
        { Language::ChineseTraditional, 0.75f }
    };

    static const std::map<Language, float> low_x = {
        { Language::English, 0.75f },
        { Language::Japanese, 0.75f },
        { Language::Spanish, 0.75f },
        { Language::French, 0.75f },
        { Language::German, 0.73f },
        { Language::Italian, 0.75f },
        { Language::Korean, 0.75f },
        { Language::ChineseSimplified, 0.75f },
        { Language::ChineseTraditional, 0.75f }
    };

    float box_y = high ? 0.42f : 0.493f;
    float box_x = high ? high_x.at(language) : low_x.at(language);
    float width = 0.9f - box_x; // max_x is always the same for all languages
    OverlayBoxScope box(stream.overlay(), { box_x, box_y, width, 0.048 });

    std::unordered_map<uint64_t, size_t> read_bids;
    size_t highest_read = 0;
    uint64_t read_value = 0;

    // read next bid multiple times since the selection arrow sometimes blocks the first digit
    for (size_t i = 0; i < 10; i++){
        VideoSnapshot screen = stream.video().snapshot();
        double screen_scale = (double)screen->width() / 1920.0;
        double vertical_padding = 5.0; // small amount of pixels so numbers do not touch the edge of the view when reading them

        ImageViewRGB32 raw_bid_image = extract_box_reference(screen, box);
        ImagePixelBox bid_bounding_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            raw_bid_image,
            [](Color pixel) {
                return (uint32_t)pixel.red() + pixel.green() + pixel.blue() < 250;
            });

        int32_t max_width = static_cast<int32_t>(raw_bid_image.width() - 1);
        int32_t max_height = static_cast<int32_t>(raw_bid_image.height() - 1);
        int32_t scaled_vertical_padding = static_cast<int32_t>(vertical_padding * screen_scale);
        int32_t left_cutoff = static_cast<int32_t>(cutoffs.at(language).first * screen_scale);
        int32_t right_cutoff = static_cast<int32_t>(cutoffs.at(language).second * screen_scale);

        ImagePixelBox cut_bid_bounding_box(
            std::max(0, std::min(max_width, static_cast<int32_t>(bid_bounding_box.min_x) + left_cutoff)),
            std::max(0, std::min(max_height, static_cast<int32_t>(bid_bounding_box.min_y) - scaled_vertical_padding)),
            std::max(0, std::min(max_width, static_cast<int32_t>(bid_bounding_box.max_x) - right_cutoff)),
            std::max(0, std::min(max_height, static_cast<int32_t>(bid_bounding_box.max_y) + scaled_vertical_padding))
            );

        uint64_t read_bid = OCR::read_number(stream.logger(), extract_box_reference(raw_bid_image, cut_bid_bounding_box));

        if (read_bids.find(read_bid) == read_bids.end()){
            read_bids[read_bid] = 0;
        }
        read_bids[read_bid] += 1;

        if (read_bids[read_bid] > highest_read){
            highest_read = read_bids[read_bid];
            read_value = read_bid;
        }
        context.wait_for(Milliseconds(20));
    }

    stream.log("Next bid: " + std::to_string(read_value));
    return read_value;
}

void AuctionFarmer::bid_on_item(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AuctionOffer offer){
    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    VideoSnapshot offer_screen = env.console.video().snapshot();

    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    PromptDialogWatcher high_detector(COLOR_RED, { 0.50, 0.40, 0.40, 0.082 });
    PromptDialogWatcher mid_detector(COLOR_PURPLE, { 0.50, 0.475, 0.40, 0.082 });
    PromptDialogWatcher low_detector(COLOR_PURPLE, { 0.50, 0.55, 0.40, 0.082 });
    OverworldWatcher overworld_detector(env.console, COLOR_BLUE);
    bool won_auction = true;
    bool auction_ongoing = true;
    int64_t current_bid = 0;

    context.wait_for_all_requests();
    while (auction_ongoing){
        int ret = wait_until(env.console, context, Milliseconds(5000), { advance_detector, high_detector, mid_detector, low_detector, overworld_detector });
        context.wait_for(Milliseconds(100));

        switch (ret){
        case 0:
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 1:
            current_bid = read_next_bid(env.console, context, LANGUAGE, true);
            pbf_press_button(context, BUTTON_A, 20, TICKS_PER_SECOND);
            break;
        case 2:
            current_bid = read_next_bid(env.console, context, LANGUAGE, false);
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

    if (won_auction){
        stats.m_auctions++;
        if (current_bid >= 0){
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

void AuctionFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

#if 0
    check_offers(env, context);
    return;

#else
    AuctionFarmer_Descriptor::Stats& stats = env.current_stats<AuctionFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);
    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    uint8_t year = MAX_YEAR;


    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        reset_auctions(env, context, true, year);
        stats.m_resets++;
        env.update_stats();
        
        bool good_offer = false;
        while (!good_offer){
            size_t npc_tries = 0;
            if (!ONE_NPC){
                pbf_move_right_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
            }

            std::vector<std::pair<AuctionOffer, ImageFloatBox>> offers = check_offers(env, context);
            for (std::pair<AuctionOffer, ImageFloatBox>& offer_pair : offers){
                AuctionOffer offer = offer_pair.first;
                if (is_good_offer(offer)){
                    try{
                        move_to_auctioneer(env, context, offer);
                    }catch (OperationFailedException& e){
                        stats.m_errors++;
                        e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

                        npc_tries++;
                        // if ONE_NPC the program already tries multiple times without change to compensate for dropped inputs
                        // at this point it is more likely to be non-recoverable
                        size_t max_npc_tries = ONE_NPC ? 1 : 3;

                        if (npc_tries < max_npc_tries){
                            VideoSnapshot screen = env.console.video().snapshot();
                            send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, e.message(), screen);
                        }else{
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to talk to the NPC!",
                                env.console
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
#endif
}




}
}
}
