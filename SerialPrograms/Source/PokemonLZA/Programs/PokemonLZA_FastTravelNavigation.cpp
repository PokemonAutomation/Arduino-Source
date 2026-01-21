/*  Fast Travel Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA_FastTravelNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{
    

const std::vector<ImageFloatBox>& FAST_TRAVEL_ARROW_BOXES(){
    static const std::vector<ImageFloatBox> boxes = {
        {0.014000, 0.242000, 0.033000, 0.066000},
        {0.014000, 0.320000, 0.033000, 0.066000},
        {0.014000, 0.398000, 0.033000, 0.066000},
        {0.014000, 0.476000, 0.033000, 0.066000},
        {0.014000, 0.554000, 0.033000, 0.066000},
        {0.014000, 0.632000, 0.033000, 0.066000},
        {0.014000, 0.710000, 0.033000, 0.066000}
    };
    return boxes;
}

const std::vector<ImageFloatBox>& FAST_TRAVEL_FILTER_ARROW_BOXES(){
    static const std::vector<ImageFloatBox> boxes = {
        {0.013000, 0.390000, 0.033000, 0.066000},
        {0.013000, 0.463000, 0.033000, 0.066000},
        {0.013000, 0.536000, 0.033000, 0.066000},
        {0.013000, 0.608000, 0.033000, 0.066000},
        {0.013000, 0.679000, 0.033000, 0.066000},
        {0.013000, 0.752000, 0.033000, 0.066000}
    };
    return boxes;
}

int get_current_selector_index(
    ConsoleHandle& console,
    const std::vector<ImageFloatBox>& arrow_boxes
){
    const ImageViewRGB32& screen = console.video().snapshot();
    for (size_t i = 0; i < arrow_boxes.size(); i++) {
        SelectionArrowWatcher arrow(
            COLOR_GREEN,
            &console.overlay(),
            SelectionArrowType::RIGHT,
            arrow_boxes[i]
        );
        if (arrow.detect(screen)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool should_navigate_down(
    const LocationItem& current_selection,
    const LocationItem& target_destination
){
    const size_t total_locations = LOCATION_ENUM_MAPPINGS().size();
    size_t down_distance = (target_destination.index + total_locations - current_selection.index) % total_locations;
    size_t up_distance = (current_selection.index + total_locations - target_destination.index) % total_locations;
    return down_distance <= up_distance;
}

bool navigate_to_destination_page_in_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    LocationItem& target_destination
){
    const size_t total_locations = LOCATION_ENUM_MAPPINGS().size();
    const size_t max_pages_to_check = ((total_locations / LocationNameReader::PAGE_SIZE) + 1) * 2; // Loop through twice to be safe

    for (size_t page = 0; page < max_pages_to_check; page++){
        LocationNameReader location_name_reader;
        std::vector<LocationItem> first_and_last_locations_on_page(2);
        GlobalThreadPools::normal_inference().run_in_parallel(
            [&](size_t index){
                OCR::StringMatchResult result = location_name_reader.read_location_name(console.video().snapshot(), console.logger(), language, index * (LocationNameReader::PAGE_SIZE - 1));
                result.clear_beyond_log10p(LocationNameOCR::MAX_LOG10P);
                result.clear_beyond_spread(LocationNameOCR::MAX_LOG10P_SPREAD);
                for (auto& item : result.results){
                    first_and_last_locations_on_page[index] = get_location_item_from_slug(item.second.token);
                }
            },
            0, 2
        );

        if (target_destination.index >= first_and_last_locations_on_page[0].index && target_destination.index <= first_and_last_locations_on_page[1].index){
            console.log("Stopping at destination page between " + first_and_last_locations_on_page[0].slug + " and " + first_and_last_locations_on_page[1].slug);
            return true;
        }
        bool navigate_down = should_navigate_down(first_and_last_locations_on_page[0], target_destination);
        if (navigate_down){
            pbf_press_button(context, BUTTON_RIGHT, 100ms, 500ms);
        }else{
            pbf_press_button(context, BUTTON_LEFT, 100ms, 500ms);
        }
        context.wait_for_all_requests();
    }
    return false;
}


int get_target_location_index_within_page(
    LocationItem& target_destination,
    const std::vector<LocationItem>& current_page_locations
){
    for (size_t index = 0; index < current_page_locations.size(); index++){
        if (current_page_locations[index].slug == target_destination.slug){
            return static_cast<int>(index);
        }
    }
    return -1;
}

bool navigate_to_destination_within_page(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    LocationItem& target_destination
){
    WallClock deadline = current_time() + 30s;
    do{
        std::vector<LocationItem> current_page_locations = read_current_page_location_items(console, language);
        int target_index = get_target_location_index_within_page(target_destination, current_page_locations);
        if (target_index == -1){
            return false;
        }
        int selector_index = get_current_selector_index(console, FAST_TRAVEL_ARROW_BOXES());
        if (selector_index == -1){
            return false;
        }
        // The target location can move due to how the game scrolls the list
        if (selector_index == target_index){
            console.log("Found destination: " + target_destination.slug);
            return true;
        }else{
            int delta = target_index - selector_index;
            for (; delta > 0; delta--){
                pbf_press_dpad(context, DPAD_DOWN, 100ms, 200ms);
            }
            for (; delta < 0; delta++){
                pbf_press_dpad(context, DPAD_UP, 100ms, 200ms);
            }   
            context.wait_for_all_requests();
        }
    } while (current_time() < deadline);
    console.log("Timeout navigating to destination: " + target_destination.slug);
    return false;
}

bool navigate_to_destination_in_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context,
    Language language,
    LocationItem& target_destination
){
    bool reached_destination_page = navigate_to_destination_page_in_fast_travel_menu(console, context, language, target_destination);
    if (!reached_destination_page){
        console.log("Unable to reach destination page for: " + target_destination.slug);
        return false;
    }
    bool found_destination = navigate_to_destination_within_page(console, context, language, target_destination);
    if (!found_destination){
        console.log("Unable to find destination on current page: " + target_destination.slug);
        return false;
    }
    return true;
}

bool set_fast_travel_menu_filter(
    ConsoleHandle& console,
    ProControllerContext& context,
    FAST_TRAVEL_FILTER filter
){
    SelectionArrowWatcher first_filter_arrow(
        COLOR_YELLOW,
        &console.overlay(),
        SelectionArrowType::RIGHT,
        FAST_TRAVEL_FILTER_ARROW_BOXES().at(0)
    );
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for(int i = 0; i < 4; i++){
                pbf_mash_button(context, BUTTON_MINUS, 1000ms);
            }
        },
        {first_filter_arrow}
    );
    switch (ret){
    case 0:
        console.log("Fast travel filter menu opened.");
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "set_fast_travel_menu_filter(): Unable to open fast travel filter menu.",
            console
        );
    }

    int target_filter_index = static_cast<int>(filter);
    WallClock deadline = current_time() + 30s;
    do {
        int selected_filter_index = get_current_selector_index(console, FAST_TRAVEL_FILTER_ARROW_BOXES());
        if (selected_filter_index == -1){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "set_fast_travel_menu_filter(): Unable to read current fast travel filter selection.",
                console
            );
        }
        if (selected_filter_index == target_filter_index){
            SelectionArrowDetector selector_arrow_on_target(
                COLOR_YELLOW,
                &console.overlay(),
                SelectionArrowType::RIGHT,
                FAST_TRAVEL_FILTER_ARROW_BOXES().at(target_filter_index)
            );
            for (size_t i = 0; i < 4; i++){
                pbf_press_button(context, BUTTON_A, 100ms, 1000ms);
                context.wait_for_all_requests();
                bool arrow_present = selector_arrow_on_target.detect(console.video().snapshot());
                if (!arrow_present){
                    console.log("Fast travel filter set.");
                    return true;
                }
            }
        }

        int delta = target_filter_index - selected_filter_index;
        for (; delta > 0; delta--){
            pbf_press_dpad(context, DPAD_DOWN, 100ms, 200ms);
        }
        for (; delta < 0; delta--){
            pbf_press_dpad(context, DPAD_UP, 100ms, 200ms);
        }
        context.wait_for_all_requests();
    } while (current_time() < deadline);
    console.log("Timeout setting fast travel filter.");
    return false;
}

bool open_fast_travel_menu(
    ConsoleHandle& console,
    ProControllerContext& context
){
    MapOverWatcher map_over(COLOR_RED, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            pbf_press_button(context, BUTTON_Y, 160ms, 1000ms);
        },
        {map_over}
    );
    switch (ret){
    case 0:
        console.log("Fast travel menu opened.");
        return true;
    default:
        console.log("Unable to open fast travel menu.");
        return false;
    }
}

std::vector<LocationItem> read_current_page_location_items(ConsoleHandle& console, Language language){
    std::vector<LocationItem> locations(LocationNameReader::PAGE_SIZE);
    LocationNameReader location_name_reader;
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            OCR::StringMatchResult result = location_name_reader.read_location_name(console.video().snapshot(), console.logger(), language, index);
            result.clear_beyond_log10p(LocationNameOCR::MAX_LOG10P);
            result.clear_beyond_spread(LocationNameOCR::MAX_LOG10P_SPREAD);
            for (auto& item : result.results){
                locations[index] = get_location_item_from_slug(item.second.token);
                console.log("Fast Travel Menu Item " + std::to_string(index) + ": " + item.second.token);
            }
        },
        0, LocationNameReader::PAGE_SIZE
    );
    if (locations.size() != LocationNameReader::PAGE_SIZE){
        console.log("Unable to read all location names in fast travel menu.");
    }
    return locations;
}

FastTravelState open_map_and_fly_to(ConsoleHandle& console, ProControllerContext& context, Language language, Location location, bool zoom_to_max, bool clear_filters){
    bool can_fast_travel = open_map(console, context, zoom_to_max, true);
    if (!can_fast_travel){
        return FastTravelState::PURSUED;
    }

    bool fast_travel_menu_opened = open_fast_travel_menu(console, context);
    if (!fast_travel_menu_opened){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_map_and_fly_to(): Unable to open fast travel menu.",
            console
        );
    }

    if (clear_filters){
        bool filters_cleared = set_fast_travel_menu_filter(console, context, FAST_TRAVEL_FILTER::ALL_TRAVEL_SPOTS);
        if (!filters_cleared){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_map_and_fly_to(): Unable to clear fast travel filters.",
                console
            );
        }
    }

    LocationItem location_item = get_location_item_from_enum(location);
    std::string target_slug = location_item.slug;
    // uint8_t target_index = location_item.index;
    
    console.log("Fast traveling to " + target_slug);

    bool success = navigate_to_destination_in_fast_travel_menu(console, context, language, location_item);
    if (!success){
        return FastTravelState::NOT_FOUND;
    }

    BlackScreenWatcher fly_confirmed(COLOR_RED);
    BlueDialogWatcher pursued_warning(COLOR_BLUE, &console.overlay(), 50ms);
    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for(int i = 0; i < 4; i++){
                pbf_mash_button(context, BUTTON_A, 1000ms);
            }
        },
        {fly_confirmed, pursued_warning}
    );
    switch (ret){
    case 0:
        console.log("Fly confirmed to " + target_slug);
        return FastTravelState::SUCCESS;
    case 1:
        console.log("Pursued by wild pokemon while flying to " + target_slug);
        return FastTravelState::PURSUED;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_map_and_fly_to(): Unable to fast travel to " + target_slug,
            console
        );
    }
    ret = wait_until(
        console, context, 30s, // 30s to load overworld on Switch 1
        {overworld}
    );
    switch(ret){
    case 0:
        console.log("Arrived at " + target_slug);
        return FastTravelState::SUCCESS;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_map_and_fly_to(): Overworld not detected",
            console
        );
    }
    return FastTravelState::NOT_FOUND;
}

}
}
}