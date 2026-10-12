#include "CommonFramework/GlobalAutoPaths.h"
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "CommonFramework/Globals.h"
#include "PokemonBDSP_OverworldMatchers.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
struct LandmarkConfig { const char* name; const char* path; ImageFloatBox target,search; double threshold,margin; };
static cv::Mat gray_image(const ImageRGB32& image){
    cv::Mat pixels((int)image.height(),(int)image.width(),CV_8UC4,
        const_cast<uint32_t*>(image.data()),image.bytes_per_row());
    cv::Mat gray; cv::cvtColor(pixels,gray,cv::COLOR_BGRA2GRAY); return gray;
}
static OverworldMatchResult match_landmarks(const ImageRGB32& frame,
    const std::vector<LandmarkConfig>& configs, size_t required, double tolerance_x, double tolerance_y, bool smooth_resize=false){
    OverworldMatchResult result;
    if (!frame) return result;
    cv::Mat gray;
    if (smooth_resize){
        // Center matching uses explicit area averaging for stable emblem edges.
        // Other matchers retain their existing ImageRGB32 resize behavior.
        cv::resize(gray_image(frame),gray,cv::Size(960,540),0,0,cv::INTER_AREA);
    }else{
        ImageRGB32 scaled=frame.scale_to(960,540);
        gray=gray_image(scaled);
    }
    for(const auto& config: configs){
        OverworldLandmarkResult landmark;landmark.name=config.name;landmark.search=config.search;
        ImageRGB32 reference(RESOURCE_PATH()+config.path);
        // Convert normalized geometry to pixels only at the image-processing boundary.
        const int template_width = std::max(1, (int)std::lround(config.target.width*gray.cols));
        const int template_height = std::max(1, (int)std::lround(config.target.height*gray.rows));
        cv::Mat templ;
        if (smooth_resize){
            cv::resize(gray_image(reference),templ,cv::Size(template_width,template_height),0,0,cv::INTER_AREA);
        }else{
            ImageRGB32 small=reference.scale_to(template_width,template_height);
            templ=gray_image(small);
        }
        cv::Rect search((int)(config.search.x*960),(int)(config.search.y*540),
            (int)(config.search.width*960),(int)(config.search.height*540));
        search &= cv::Rect(0,0,960,540);
        if(search.width<templ.cols || search.height<templ.rows){result.landmarks.push_back(landmark);continue;}
        cv::Mat scores;cv::matchTemplate(gray(search),templ,scores,cv::TM_CCOEFF_NORMED);
        cv::Point location;cv::minMaxLoc(scores,nullptr,&landmark.score,nullptr,&location);
        cv::Mat competitors=scores.clone();
        const int peak_radius_x = (int)std::lround(0.015625*gray.cols);
        const int peak_radius_y = (int)std::lround((1.0/36)*gray.rows);
        cv::Rect exclusion=cv::Rect(location.x-peak_radius_x,location.y-peak_radius_y,
            2*peak_radius_x+1,2*peak_radius_y+1)&cv::Rect(0,0,scores.cols,scores.rows);
        competitors(exclusion).setTo(-1);
        double runner_up;cv::minMaxLoc(competitors,nullptr,&runner_up);
        landmark.margin=landmark.score-runner_up;
        location+=search.tl();
        landmark.dx=(double)location.x/gray.cols-config.target.x;
        landmark.dy=(double)location.y/gray.rows-config.target.y;
        landmark.accepted=landmark.score>=config.threshold && landmark.margin>=config.margin;
        landmark.found={(double)location.x/960,(double)location.y/540,(double)templ.cols/960,(double)templ.rows/540};
        result.landmarks.push_back(landmark);
    }
    if(required==1 && result.landmarks.size()==1){
        result.located=result.landmarks[0].accepted;result.dx=result.landmarks[0].dx;result.dy=result.landmarks[0].dy;
    }else{
        size_t best_support=0; bool ambiguous=false;
        for(const auto& candidate:result.landmarks){
            if(!candidate.accepted)continue;
            size_t support=0;double dx=0,dy=0;
            for(const auto& other:result.landmarks){
                if(other.accepted && std::abs(candidate.dx-other.dx)<=1.0/192 && std::abs(candidate.dy-other.dy)<=1.0/108){
                    support++;dx+=other.dx;dy+=other.dy;
                }
            }
            if(support<required)continue;
            dx/=support;dy/=support;
            if(support>best_support){best_support=support;result.dx=dx;result.dy=dy;ambiguous=false;}
            else if(support==best_support && (std::abs(dx-result.dx)>1.0/192 || std::abs(dy-result.dy)>1.0/108)) ambiguous=true;
        }
        result.located=best_support>=required && !ambiguous;
    }
    result.at_target=result.located && std::abs(result.dx)<=tolerance_x && std::abs(result.dy)<=tolerance_y;
    return result;
}
OverworldMatchResult match_poketch_landmarks(const ImageRGB32& frame){
    // Same cropped reference emblems and local search windows as production.
    OverworldMatchResult fine = match_landmarks(frame,{
        {"Mart","PokemonBDSP/Autostory/PoketchInteraction/Mart.png",{0.768750000000,0.044444444444,0.079166666667,0.088888888889},{598.0/960,0,356.0/960,212.0/540},.90,0},
        {"Center","PokemonBDSP/Autostory/PoketchInteraction/Center.png",{0.900000000000,0.748148148148,0.075000000000,0.137037037037},{724.0/960,264.0/540,236.0/960,276.0/540},.90,0},
        {"School","PokemonBDSP/Autostory/PoketchInteraction/School.png",{0.006250000000,0.755555555556,0.097916666667,0.133333333333},{0,268.0/540,240.0/960,272.0/540},.90,0}
    },2,7.0/960,7.0/540);
    if (fine.located) return fine;
    // Coarse acquisition searches the whole frame. Agreement and a peak margin
    // guard against confusing the Mart/Center emblems with another building.
    OverworldMatchResult coarse = match_landmarks(frame,{
        {"Mart","PokemonBDSP/Autostory/PoketchInteraction/Mart.png",{0.768750000000,0.044444444444,0.079166666667,0.088888888889},{0,0,1,1},.93,.12},
        {"Center","PokemonBDSP/Autostory/PoketchInteraction/Center.png",{0.900000000000,0.748148148148,0.075000000000,0.137037037037},{0,0,1,1},.93,.12},
        {"School","PokemonBDSP/Autostory/PoketchInteraction/School.png",{0.006250000000,0.755555555556,0.097916666667,0.133333333333},{0,0,1,1},.92,.40}
    },2,7.0/960,7.0/540);
    if (!coarse.located) {
        // Acquisition may expose just one building. School keeps its strong
        // spatial margin; Mart/Center require a higher score than pair matching.
        // A single anchor guides coarse movement only, never interaction.
        const OverworldLandmarkResult* anchor = nullptr;
        for (const auto& landmark : coarse.landmarks) {
            const bool strong = landmark.accepted && (landmark.name == "School" ||
                (landmark.score >= .95 && landmark.margin >= .12));
            if (strong && (!anchor || landmark.score > anchor->score)) anchor = &landmark;
        }
        if (anchor) {
            bool contradictory = false;
            for (const auto& other : coarse.landmarks)
                if (other.accepted && (std::abs(other.dx - anchor->dx) > 1.0/192 || std::abs(other.dy - anchor->dy) > 1.0/108))
                    contradictory = true;
            if (!contradictory) {
                coarse.located = true;
                coarse.dx = anchor->dx;
                coarse.dy = anchor->dy;
            }
        }
    }
    coarse.coarse = true;
    // A coarse match may guide movement, but never authorize interaction.
    // Final position must pass the normal local two-landmark confirmation.
    coarse.at_target = false;
    return coarse;
}
OverworldMatchResult match_oreburgh_mart(const ImageRGB32& frame){
    return match_landmarks(frame,{{"Mart emblem","PokemonBDSP/Autostory/OreburghMart/MartEmblem.png",
        {0.502083333333,0.235185185185,0.056250000000,0.079629629630},{0,0,1,360.0/540},.93,.10}},1,1.0/128,1.0/72);
}
OverworldMatchResult match_jubilife_center_entrance(const ImageRGB32& frame){
    // Native lighting-test captures avoid resizing artifacts in the reference.
    // The emblem is about 5.73% of screen width left of the player at approach:
    // shift its target right to center the door, preserving the approach height.
    OverworldMatchResult clear = match_landmarks(frame,{{"Center entrance",
        "PokemonBDSP/Autostory/JubilifeCenter/CenterEmblem.png",
        {0.462500000000,0.109259259259,0.072916666667,0.111111111111},{0,0,1,1},.90,.20}},1,1.0/128,1.0/72,true);
    OverworldMatchResult shadow = match_landmarks(frame,{{"Center entrance",
        "PokemonBDSP/Autostory/JubilifeCenter/CenterEmblemShadow.png",
        {0.462500000000,0.109259259259,0.072916666667,0.111111111111},{0,0,1,1},.90,.20}},1,1.0/128,1.0/72,true);
    // Lighting alternatives must identify the same position when both pass.
    if (clear.located && shadow.located &&
        (std::abs(clear.dx-shadow.dx)>1.0/192 || std::abs(clear.dy-shadow.dy)>1.0/108)){
        clear.located = false;
        clear.at_target = false;
        return clear;
    }
    if (shadow.located && (!clear.located || shadow.landmarks[0].score>clear.landmarks[0].score))
        return shadow;
    return clear;
}
OverworldMatchResult match_route201_potion_exit(const ImageRGB32& frame){
    // The sign face is stable across lighting; its post and surrounding grass are not.
    // Path shadows change shape, so validate the corner against four lighting states.
    // Each alternative still requires the sign and path to agree on position.
    const char* corners[]={
        "PokemonBDSP/Autostory/Route201PotionExit/PathCornerHour0.png",
        "PokemonBDSP/Autostory/Route201PotionExit/PathCornerHour9.png",
        "PokemonBDSP/Autostory/Route201PotionExit/PathCornerHour12.png",
        "PokemonBDSP/Autostory/Route201PotionExit/PathCornerHour17.png"
    };
    OverworldMatchResult best;
    double best_score=-1;
    for (const char* corner : corners){
        auto candidate=match_landmarks(frame,{
            {"Route sign face","PokemonBDSP/Autostory/Route201PotionExit/RouteSignFace.png",
                {0.292708333333,0.470370370370,0.109375,0.072222222222},{0,0,1,1},.94,.20},
            {"Path corner",corner,
                {0.202083333333,0.648148148148,0.133333333333,0.174074074074},{0,0,1,1},.90,.10}
        },2,1.0/128,1.0/72,true);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>1.0/192 || std::abs(best.dy-candidate.dy)>1.0/108)){
            best.located=false;
            best.at_target=false;
            return best;
        }
        if (candidate.landmarks.size()!=2) continue;
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)){
            best=std::move(candidate);
            best_score=score;
        }
    }
    return best;
}
OverworldMatchResult match_sandgem_junction(const ImageRGB32& frame){
    // Building emblems contain no localized words. Ignore NPCs and door lights.
    auto result = match_landmarks(frame,{
        {"Center emblem","PokemonBDSP/Autostory/SandgemJunction/Center.png",
            {0.171875,0.2,0.058333333333,0.092592592593},{0,0,1,1},.90,.08},
        {"Mart emblem","PokemonBDSP/Autostory/SandgemJunction/Mart.png",
            {0.726041666667,0.222222222222,0.0625,0.088888888889},{0,0,1,1},.90,.08}
    },2,1.0/128,1.0/72,true);
    if (result.located || result.landmarks.size()!=2) return result;
    const auto& center=result.landmarks[0];
    const auto& mart=result.landmarks[1];
    const double dy=(center.dy+mart.dy)/2;
    const double horizontal_disagreement=center.dx-mart.dx;
    // At a different approach height, perspective changes the buildings' spacing.
    // Trust matching height offsets for vertical acquisition only. Bound the
    // horizontal discrepancy and require its direction to agree with perspective:
    // scenery above target contracts, scenery below target expands.
    if (center.accepted && mart.accepted &&
        std::abs(center.dy-mart.dy)<=1.0/108 &&
        std::abs(dy)>1.0/72 && std::abs(dy)<=0.20 &&
        std::abs(horizontal_disagreement)<=0.04 &&
        std::abs(horizontal_disagreement)<=0.35*std::abs(dy)+1.0/192 &&
        horizontal_disagreement*dy<=0){
        result.located=true;
        result.coarse=true;
        result.dx=(center.dx+mart.dx)/2;
        result.dy=dy;
        // Acquisition may move vertically; it never confirms the route endpoint.
        result.at_target=false;
    }
    return result;
}
OverworldMatchResult match_route202_grass_exit(const ImageRGB32& frame){
    // Route marker and upper path bend avoid the player, NPCs, and localized text.
    // Morning tree shadows need a second corner reference. Each alternative
    // must still agree with the sign; neither can confirm the destination alone.
    const char* corners[]={
        "PokemonBDSP/Autostory/Route202GrassExit/PathCorner.png",
        "PokemonBDSP/Autostory/Route202GrassExit/PathCornerMorning.png"
    };
    OverworldMatchResult best;
    double best_score=-1;
    for (const char* corner : corners){
        auto candidate=match_landmarks(frame,{
            {"Route 202 sign","PokemonBDSP/Autostory/Route202GrassExit/RouteSign.png",
                {0.59375,0.194444444444,0.091666666667,0.059259259259},{0,0,1,1},.94,.20},
            {"Path corner",corner,
                {0.405208333333,0.331481481481,0.1375,0.062962962963},{0,0,1,1},.90,.08}
        },2,1.0/128,1.0/72,true);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>1.0/192 || std::abs(best.dy-candidate.dy)>1.0/108)){
            best.located=false;
            best.at_target=false;
            return best;
        }
        if (candidate.landmarks.size()!=2) continue;
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)){
            best=std::move(candidate);
            best_score=score;
        }
    }
    return best;
}
OverworldMatchResult match_jubilife_street_approach(const ImageRGB32& frame){
    // Static building/fence geometry excludes people and localized signs.
    return match_landmarks(frame,{
        {"Door","PokemonBDSP/Autostory/JubilifeStreet/Door.png",
            {0.677083333333,0.439814814815,0.213541666667,0.162037037037},{0,0,1,1},.90,.04},
        {"Facade","PokemonBDSP/Autostory/JubilifeStreet/Facade.png",
            {0.583333333333,0.138888888889,0.322916666667,0.250000000000},{0,0,1,1},.90,.04}
    },2,1.0/128,1.0/72,true);
}
OverworldMatchResult match_jubilife_plaza_approach(const ImageRGB32& frame){
    // Static building/fence geometry excludes people and localized signs.
    return match_landmarks(frame,{
        {"Roof","PokemonBDSP/Autostory/JubilifePlaza/Roof.png",
            {0.640625000000,0.069444444444,0.260416666667,0.189814814815},{0,0,1,1},.90,.04},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/Fence.png",
            {0.041666666667,0.319444444444,0.169270833333,0.120370370370},{0,0,1,1},.90,.04}
    },2,1.0/128,1.0/72,true);
}
const std::vector<OverworldMatcher>& overworld_matchers(){
    static const std::vector<OverworldMatcher> matchers={
        {"jubilife-street-approach","Jubilife street before Galactic plaza",match_jubilife_street_approach},
        {"jubilife-plaza-approach","Jubilife Galactic plaza approach",match_jubilife_plaza_approach},
        {"poketch-landmarks","Jubilife Poketch landmarks",match_poketch_landmarks},
        {"oreburgh-mart","Oreburgh Mart emblem",match_oreburgh_mart},
        {"jubilife-center","Jubilife Pokemon Center entrance",match_jubilife_center_entrance},
        {"route201-potion-exit","Route 201 after Potion pickup",match_route201_potion_exit},
        {"sandgem-junction","Sandgem Center/Mart junction",match_sandgem_junction},
        {"route202-grass-exit","Route 202 after catching: grass exit",match_route202_grass_exit},
    };
    return matchers;
}
}}}
