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
OverworldMatchResult match_poketch_landmarks(const ImageRGB32& frame);
OverworldMatchResult match_oreburgh_mart(const ImageRGB32& frame);
OverworldMatchResult match_jubilife_center_entrance(const ImageRGB32& frame);
OverworldMatchResult match_route201_potion_exit(const ImageRGB32& frame);
OverworldMatchResult match_sandgem_junction(const ImageRGB32& frame);
OverworldMatchResult match_route202_grass_exit(const ImageRGB32& frame);
}}}
