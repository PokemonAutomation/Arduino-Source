#pragma once
#include <functional>
#include <string>
#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
struct OverworldLandmarkResult {
    std::string name;
    double score = 0, margin = 0;
    // Signed fractions of screen width/height, independent of capture resolution.
    double dx = 0, dy = 0;
    bool accepted = false;
    ImageFloatBox search, found;
};
struct OverworldMatchResult {
    std::vector<OverworldLandmarkResult> landmarks;
    bool located = false, at_target = false;
    bool coarse = false;
    // Signed fractions of screen width/height, independent of capture resolution.
    double dx = 0, dy = 0;
};
struct OverworldMatcher {
    std::string slug, name;
    std::function<OverworldMatchResult(const ImageRGB32&)> match;
};
// Register additional functions here; each owns its landmarks and coordinate system.
const std::vector<OverworldMatcher>& overworld_matchers();
OverworldMatchResult match_route204_north_approach(const ImageRGB32& frame);
OverworldMatchResult match_teighler_alcove_corner(const ImageRGB32& frame);
OverworldMatchResult match_route204_north_stairs(const ImageRGB32& frame);
OverworldMatchResult match_floaroma_center_entrance(const ImageRGB32& frame);
OverworldMatchResult match_floaroma_house_entrance(const ImageRGB32& frame);
OverworldMatchResult match_ravaged_path_south_entrance(const ImageRGB32& frame);
OverworldMatchResult match_route204_sign(const ImageRGB32& frame);
OverworldMatchResult match_windworks_sign_approach(const ImageRGB32& frame);
// Interior only: offsets from the player's red cap to the grunt's cyan hair.
// Positive dy > .025 means the player is above him; otherwise below/level.
OverworldMatchResult match_windworks_interior_grunt(const ImageRGB32& frame);
OverworldMatchResult match_route205_bridge(const ImageRGB32& frame);
OverworldMatchResult match_route205_bank_tree(const ImageRGB32& frame);
OverworldMatchResult match_route205_zackary(const ImageRGB32& frame);
OverworldMatchResult match_route205_kelsey(const ImageRGB32& frame);
OverworldMatchResult match_route205_forest_entrance(const ImageRGB32& frame);
// Northern sign: align primarily westward, then correct height below the flower bed.
OverworldMatchResult match_route204_floaroma_turn(const ImageRGB32& frame);
OverworldMatchResult match_jubilife_street_approach(const ImageRGB32& frame);
OverworldMatchResult match_jubilife_plaza_approach(const ImageRGB32& frame);
OverworldMatchResult match_poketch_landmarks(const ImageRGB32& frame);
OverworldMatchResult match_oreburgh_mart(const ImageRGB32& frame);
OverworldMatchResult match_jubilife_center_entrance(const ImageRGB32& frame);
OverworldMatchResult match_eterna_center_entrance(const ImageRGB32& frame);
OverworldMatchResult match_hearthome_center_entrance(const ImageRGB32& frame);
OverworldMatchResult match_hearthome_center_interior(const ImageRGB32& frame);
OverworldMatchResult match_hearthome_contest_fantina(const ImageRGB32& frame);
OverworldMatchResult match_route208_stair_top(const ImageRGB32& frame);
OverworldMatchResult match_route208_hannah_departure(const ImageRGB32& frame);
OverworldMatchResult match_eterna_gym_entrance(const ImageRGB32& frame);
OverworldMatchResult match_galactic_hq_cut(const ImageRGB32& frame);
OverworldMatchResult match_galactic_hq_jupiter(const ImageRGB32& frame);
OverworldMatchResult match_eterna_cycle_shop_entrance(const ImageRGB32& frame);
OverworldMatchResult match_eterna_cycle_shop_interior(const ImageRGB32& frame);
OverworldMatchResult match_eterna_cycle_shop_reward(const ImageRGB32& frame);
OverworldMatchResult match_eterna_bike_riding_card(const ImageRGB32& frame);
OverworldMatchResult match_eterna_cycle_shop_counter(const ImageRGB32& frame);
OverworldMatchResult match_cycling_road_bridge_approach(const ImageRGB32& frame);
OverworldMatchResult match_cycling_road_south_gate_interior(const ImageRGB32& frame);
OverworldMatchResult match_galactic_hq_exterior(const ImageRGB32& frame);
OverworldMatchResult match_eterna_center_departure(const ImageRGB32& frame);
OverworldMatchResult match_eterna_gym_statues(const ImageRGB32& frame);
// Two unique, agreeing landmarks close to the doorway; this confidence only
// authorizes a watched entry attempt, never proves the interior location.
double overworld_entry_confidence(const OverworldMatchResult& result,
    double tolerance_x=1.0/50, double tolerance_y=1.0/27, double minimum_margin=.15);
OverworldMatchResult match_eterna_underground_house(const ImageRGB32& frame);
OverworldMatchResult match_eterna_underground_man(const ImageRGB32& frame);
OverworldMatchResult match_route201_potion_exit(const ImageRGB32& frame);
OverworldMatchResult match_sandgem_junction(const ImageRGB32& frame);
OverworldMatchResult match_route202_grass_exit(const ImageRGB32& frame);
}}}
