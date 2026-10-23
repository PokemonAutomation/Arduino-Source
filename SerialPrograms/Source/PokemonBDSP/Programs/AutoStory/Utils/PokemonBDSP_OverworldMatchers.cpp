#include "CommonFramework/GlobalAutoPaths.h"
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "CommonFramework/Globals.h"
#include "PokemonBDSP_OverworldMatchers.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
enum class LandmarkFeatures { GRAYSCALE, LOCAL_CONTRAST, BLUE_FRACTION };
struct LandmarkConfig { const char* name; const char* path; ImageFloatBox target,search; double threshold,margin; LandmarkFeatures features=LandmarkFeatures::GRAYSCALE; };
// Screen-relative half-widths of the accepted position box, and how closely
// two landmarks (or reference variants) must agree to be trusted together.
static constexpr double POSITION_TOLERANCE_X = 1.0/50;
static constexpr double POSITION_TOLERANCE_Y = 1.0/54;
static constexpr double AGREEMENT_X = 1.0/64;
static constexpr double AGREEMENT_Y = 1.0/54;
static cv::Mat gray_image(const ImageRGB32& image){
    cv::Mat pixels((int)image.height(),(int)image.width(),CV_8UC4,
        const_cast<uint32_t*>(image.data()),image.bytes_per_row());
    cv::Mat gray; cv::cvtColor(pixels,gray,cv::COLOR_BGRA2GRAY); return gray;
}
// Opt-in for selected outdoor structural anchors. Divide out slow local lighting
// changes so a moving cloud does not alter the printed sign/emblem structure.
static cv::Mat normalize_landmark_illumination(const cv::Mat& gray){
    cv::Mat pixels,background,normalized;
    gray.convertTo(pixels,CV_32F);
    cv::GaussianBlur(pixels,background,cv::Size(0,0),3.0);
    cv::divide(pixels,background+1.0,normalized);
    cv::Mat structure;
    normalized.convertTo(structure,CV_8U,128.0);
    return structure;
}
// White Pokeball versus yellow fascia is a color-ratio boundary. A cast
// shadow changes their brightness, so compare blue fraction rather than luma.
static cv::Mat landmark_blue_fraction(const ImageRGB32& image){
    cv::Mat pixels((int)image.height(),(int)image.width(),CV_8UC4,
        const_cast<uint32_t*>(image.data()),image.bytes_per_row());
    cv::Mat small,floating;
    cv::resize(pixels,small,cv::Size(960,540),0,0,cv::INTER_AREA);
    small.convertTo(floating,CV_32F);
    std::vector<cv::Mat> channels;cv::split(floating,channels);
    cv::Mat ratio,structure;
    cv::divide(channels[0],channels[0]+channels[1]+channels[2]+3.0,ratio);
    ratio.convertTo(structure,CV_8U,768.0);
    return structure;
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
    cv::Mat normalized_gray,chromaticity;
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
        cv::Mat scores;
        if (config.features==LandmarkFeatures::BLUE_FRACTION){
            if (chromaticity.empty()) chromaticity=landmark_blue_fraction(frame);
            cv::matchTemplate(chromaticity(search),templ,scores,cv::TM_CCOEFF_NORMED);
        }else if (config.features==LandmarkFeatures::LOCAL_CONTRAST){
            if (normalized_gray.empty()) normalized_gray=normalize_landmark_illumination(gray);
            // These opt-in references are normalized from their full source
            // frame before cropping, avoiding false edges at template borders.
            cv::matchTemplate(normalized_gray(search),templ,scores,cv::TM_CCOEFF_NORMED);
        }else{
            cv::matchTemplate(gray(search),templ,scores,cv::TM_CCOEFF_NORMED);
        }
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
                if(other.accepted && std::abs(candidate.dx-other.dx)<=AGREEMENT_X && std::abs(candidate.dy-other.dy)<=AGREEMENT_Y){
                    support++;dx+=other.dx;dy+=other.dy;
                }
            }
            if(support<required)continue;
            dx/=support;dy/=support;
            if(support>best_support){best_support=support;result.dx=dx;result.dy=dy;ambiguous=false;}
            else if(support==best_support && (std::abs(dx-result.dx)>AGREEMENT_X || std::abs(dy-result.dy)>AGREEMENT_Y)) ambiguous=true;
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
    const ImageFloatBox target{0.502083333333,0.235185185185,0.056250000000,0.079629629630};
    auto original=match_landmarks(frame,{{"Mart emblem","PokemonBDSP/Autostory/OreburghMart/MartEmblem.png",
        target,{0,0,1,360.0/540},.93,.10}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y);
    // The recorded daytime roof appearance narrowly missed the old template.
    // Preserve both confidence gates and reject disagreement between variants.
    auto recorded=match_landmarks(frame,{{"Mart emblem","PokemonBDSP/Autostory/OreburghMart/MartEmblemRecorded.png",
        target,{0,0,1,360.0/540},.93,.10}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y);
    if (original.located && recorded.located &&
        (std::abs(original.dx-recorded.dx)>AGREEMENT_X || std::abs(original.dy-recorded.dy)>AGREEMENT_Y)){
        original.located=false;original.at_target=false;return original;
    }
    if ((recorded.located && !original.located) || (recorded.located==original.located &&
        recorded.landmarks[0].score>original.landmarks[0].score)) return recorded;
    return original;
}
OverworldMatchResult match_jubilife_center_entrance(const ImageRGB32& frame){
    // Native lighting-test captures avoid resizing artifacts in the reference.
    // The emblem is about 5.73% of screen width left of the player at approach:
    // shift its target right to center the door, preserving the approach height.
    OverworldMatchResult clear = match_landmarks(frame,{{"Center entrance",
        "PokemonBDSP/Autostory/JubilifeCenter/CenterEmblem.png",
        {0.462500000000,0.109259259259,0.072916666667,0.111111111111},{0,0,1,1},.90,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
    OverworldMatchResult shadow = match_landmarks(frame,{{"Center entrance",
        "PokemonBDSP/Autostory/JubilifeCenter/CenterEmblemShadow.png",
        {0.462500000000,0.109259259259,0.072916666667,0.111111111111},{0,0,1,1},.90,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
    // Lighting alternatives must identify the same position when both pass.
    if (clear.located && shadow.located &&
        (std::abs(clear.dx-shadow.dx)>AGREEMENT_X || std::abs(clear.dy-shadow.dy)>AGREEMENT_Y)){
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
        },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>AGREEMENT_X || std::abs(best.dy-candidate.dy)>AGREEMENT_Y)){
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
    },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
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
        std::abs(center.dy-mart.dy)<=AGREEMENT_Y &&
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
        },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>AGREEMENT_X || std::abs(best.dy-candidate.dy)>AGREEMENT_Y)){
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
// Lighting alternatives describe one physical landmark, not extra votes.
// Select one reference per landmark before requiring the two positions to agree.
static OverworldMatchResult match_lighting_pair(const ImageRGB32& frame,
    const std::vector<LandmarkConfig>& variants, const char* first, const char* second,
    double tolerance_x=POSITION_TOLERANCE_X, bool allow_vertical_acquisition=false, double agreement_x=AGREEMENT_X){
    auto candidates=match_landmarks(frame,variants,variants.size()+1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
    OverworldMatchResult result;
    for (const char* name : {first,second}){
        const OverworldLandmarkResult* best=nullptr;
        for (const auto& candidate : candidates.landmarks){
            if (candidate.name!=name) continue;
            if (!best || (candidate.accepted && !best->accepted) ||
                (candidate.accepted==best->accepted && candidate.score>best->score)) best=&candidate;
        }
        if (!best) return result;
        result.landmarks.push_back(*best);
        auto& chosen=result.landmarks.back();
        // Similar strong references must not disagree about location.
        for (const auto& candidate : candidates.landmarks){
            if (candidate.name==name && candidate.accepted &&
                candidate.score>=chosen.score-0.02 &&
                (std::abs(candidate.dx-chosen.dx)>AGREEMENT_X ||
                 std::abs(candidate.dy-chosen.dy)>AGREEMENT_Y)) chosen.accepted=false;
        }
    }
    const auto& a=result.landmarks[0];const auto& b=result.landmarks[1];
    result.located=a.accepted && b.accepted &&
        std::abs(a.dx-b.dx)<=agreement_x && std::abs(a.dy-b.dy)<=AGREEMENT_Y;
    result.dx=(a.dx+b.dx)/2;result.dy=(a.dy+b.dy)/2;
    result.at_target=result.located &&
        std::abs(a.dx-b.dx)<=AGREEMENT_X &&        std::abs(result.dx)<=tolerance_x && std::abs(result.dy)<=POSITION_TOLERANCE_Y;
    const double spacing_difference=a.dx-b.dx;
    // At the street approach height, perspective can change column spacing.
    // Agreeing vertical offsets may guide an upward correction, but cannot
    // confirm the destination or authorize a horizontal correction yet.
    if (!result.located && allow_vertical_acquisition && a.accepted && b.accepted &&
        std::abs(a.dy-b.dy)<=AGREEMENT_Y && std::abs(result.dy)>POSITION_TOLERANCE_Y && std::abs(result.dy)<=0.20 &&
        std::abs(spacing_difference)<=0.04 &&
        std::abs(spacing_difference)<=0.35*std::abs(result.dy)+1.0/192 && spacing_difference*result.dy<=0){
        result.located=true;
        result.coarse=true;
        result.at_target=false;
    }
    return result;
}
OverworldMatchResult match_eterna_center_entrance(const ImageRGB32& frame){
    // Native screenshot-20261016-214300767696 is the intended doorway pose.
    // The Pokeball is primary. Sign and bush assist when visible, but cloud
    // shadows must not prevent arrival from a strong, unique emblem match.
    OverworldMatchResult best;
    double best_score=-1;
    bool best_primary_only=false;
    // Recorded evening/daytime appearances retain the same destination and gates.
    for (int appearance=0;appearance<5;appearance++){
        for (double scale : {.90,.95,1.0,1.05,1.10}){
            std::vector<LandmarkConfig> configs={
                {"Center Pokeball","PokemonBDSP/Autostory/EternaCenter/BallTarget.png",
                    {904.0/1920,144.0/1080,112.0/1920,104.0/1080},{0,0,1,.65},.93,.20},
                {"Right sign","PokemonBDSP/Autostory/EternaCenter/RightSignTarget.png",
                    {1762.0/1920,304.0/1080,128.0/1920,138.0/1080},{0,0,1,.85},.93,.20},
                {"Lower left bush","PokemonBDSP/Autostory/EternaCenter/BushTarget.png",
                    {248.0/1920,854.0/1080,180.0/1920,146.0/1080},{0,.35,1,.65},.93,.20}
            };
            if (appearance==1 || appearance==2){
                configs={
                    {"Center Pokeball",appearance==1 ? "PokemonBDSP/Autostory/EternaCenter/BallEvening1.png" : "PokemonBDSP/Autostory/EternaCenter/BallEvening2.png",
                        {920.0/1920,154.0/1080,82.0/1920,84.0/1080},{0,0,1,.65},.93,.20},
                    {"Right sign",appearance==1 ? "PokemonBDSP/Autostory/EternaCenter/RightSignEvening1.png" : "PokemonBDSP/Autostory/EternaCenter/RightSignEvening2.png",
                        {1770.0/1920,310.0/1080,116.0/1920,63.0/1080},{0,0,1,.85},.93,.20},
                    {"Lower left bush",appearance==1 ? "PokemonBDSP/Autostory/EternaCenter/BushEvening1.png" : "PokemonBDSP/Autostory/EternaCenter/BushEvening2.png",
                        {248.0/1920,932.0/1080,180.0/1920,68.0/1080},{0,.35,1,.65},.93,.20}
                };
            }
            if (appearance>=3){
                // Recorded daylight changes the bush/planter contrast. These
                // crops retain the original doorway geometry and all three votes.
                configs[0].path=appearance==3 ? "PokemonBDSP/Autostory/EternaCenter/BallDaytime1.png" : "PokemonBDSP/Autostory/EternaCenter/BallDaytime2.png";
                configs[1].path=appearance==3 ? "PokemonBDSP/Autostory/EternaCenter/RightSignDaytime1.png" : "PokemonBDSP/Autostory/EternaCenter/RightSignDaytime2.png";
                configs[2].path=appearance==3 ? "PokemonBDSP/Autostory/EternaCenter/BushDaytime1.png" : "PokemonBDSP/Autostory/EternaCenter/BushDaytime2.png";
            }
            for (auto& config : configs){
                const double w=config.target.width,h=config.target.height;
                config.target.width*=scale;config.target.height*=scale;
                config.target.x+=(w-config.target.width)/2;
                config.target.y+=(h-config.target.height)/2;
            }
            auto candidate=match_landmarks(frame,configs,3,1.0/128,POSITION_TOLERANCE_Y,true);
            if (candidate.located) {
                double score=1;
                for (const auto& landmark:candidate.landmarks) score=std::min(score,landmark.score);
                if (!best.located || best_primary_only || score>best_score){best=candidate;best_score=score;best_primary_only=false;}
            } else if (!best.located || best_primary_only) {
                // Optional sign/bush can disappear under clouds or at frame edges.
                // Retain the existing stricter primary-emblem confidence gate.
                const auto& ball=candidate.landmarks[0];
                if (ball.accepted && ball.score>=.95 &&
                    (!best.located || ball.score>best_score)){
                    best=candidate;best.located=true;best.coarse=false;
                    best.dx=ball.dx;best.dy=ball.dy;
                    best.at_target=std::abs(best.dx)<=1.0/128 && std::abs(best.dy)<=POSITION_TOLERANCE_Y;
                    best_score=ball.score;best_primary_only=true;
                } else if (best.landmarks.empty()) best=candidate;
            }
        }
    }
    return best;
}
OverworldMatchResult match_route208_stair_top(const ImageRGB32& frame){
    // Native screenshot 20261022-213928168268: centered above the east stairs.
    // The isolated upper rock and lower rock pair must both agree before descent.
    OverworldMatchResult best;
    double best_score=-1;
    // Stationary hourly captures supply lighting appearances, not new goals.
    // Both physical rock groups still need the same confidence and agreement.
    const char* references[][2]={
        {"PokemonBDSP/Autostory/Route208Stairs/UpperRock.png","PokemonBDSP/Autostory/Route208Stairs/LowerRocks.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour0Frame1Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour0Frame1Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour4Frame1Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour4Frame1Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour17Frame1Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour17Frame1Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour11Frame3Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour11Frame3Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour15Frame1Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour15Frame1Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour11Frame5Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour11Frame5Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Hour15Frame3Upper.png","PokemonBDSP/Autostory/Route208Stairs/Hour15Frame3Lower.png"},
    };
    for (const auto& reference:references){
    for (double scale : {.90,.95,1.0,1.05,1.10}){
        std::vector<LandmarkConfig> configs={
            {"Stair-top upper rock",reference[0],
                {170.0/1920,350.0/1080,120.0/1920,120.0/1080},{0,0,1,1},.93,.20},
            {"Stair-top lower rock pair",reference[1],
                {240.0/1920,425.0/1080,130.0/1920,180.0/1080},{0,0,1,1},.93,.20}
        };
        for (auto& config:configs){
            const double w=config.target.width,h=config.target.height;
            config.target.width*=scale;config.target.height*=scale;
            config.target.x+=(w-config.target.width)/2;
            config.target.y+=(h-config.target.height)/2;
        }
        auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
        double score=1;
        for (const auto& landmark:candidate.landmarks) score=std::min(score,landmark.score);
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)){
            best=std::move(candidate);best_score=score;
        }
    }
    }
    if (best.located) return best;
    const char* approaches[][2]={
        {"PokemonBDSP/Autostory/Route208Stairs/Approach0507Upper.png","PokemonBDSP/Autostory/Route208Stairs/Approach0507Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Approach0538Upper.png","PokemonBDSP/Autostory/Route208Stairs/Approach0538Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/Approach0549Upper.png","PokemonBDSP/Autostory/Route208Stairs/Approach0549Lower.png"},
        // The eastern cliff pose changes rock perspective and the local shadow
        // pattern. These real-run appearances guide acquisition only: retain
        // .92 acquisition NCC/.20 uniqueness and two-rock offset agreement.
        // Arrival still requires the original .93 strict reference bank.
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive01Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive01Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive02Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive02Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive03Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive03Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive04Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive04Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive05Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive05Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive06Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive06Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive07Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive07Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive08Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive08Lower.png"},
        {"PokemonBDSP/Autostory/Route208Stairs/ApproachLive09Upper.png","PokemonBDSP/Autostory/Route208Stairs/ApproachLive09Lower.png"},
    };
    for (const auto& reference:approaches){
        for (double scale : {.90,.925,.95,.975,1.0,1.025,1.05,1.075,1.10}){
            std::vector<LandmarkConfig> configs={
                {"Stair-top approach upper rock",reference[0],
                    {170.0/1920,350.0/1080,120.0/1920,120.0/1080},{0,0,.50,.85},.92,.20},
                {"Stair-top approach lower rock pair",reference[1],
                    {240.0/1920,425.0/1080,130.0/1920,180.0/1080},{0,0,.50,.85},.92,.20}
            };
            for (auto& config:configs){
                const double w=config.target.width,h=config.target.height;
                config.target.width*=scale;config.target.height*=scale;
                config.target.x+=(w-config.target.width)/2;
                config.target.y+=(h-config.target.height)/2;
            }
            auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
            candidate.coarse=true;candidate.at_target=false;
            const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
            if (candidate.located && (!best.located || score>best_score)){
                best=std::move(candidate);best_score=score;
            }
        }
    }
    return best;
}
OverworldMatchResult match_route208_hannah_departure(const ImageRGB32& frame){
    OverworldMatchResult best;
    double best_score=-1;
    for (int appearance : {0,865,930,965,978}){
        for (double scale : {.90,.95,1.0,1.05,1.10}){
            std::vector<LandmarkConfig> configs={
                {"Field stair foot","PokemonBDSP/Autostory/Route208Hannah/StairFoot.png",
                    {225.0/1920,275.0/1080,250.0/1920,110.0/1080},{0,0,1,1},.93,.20,LandmarkFeatures::LOCAL_CONTRAST},
                {"Field sign frame","PokemonBDSP/Autostory/Route208Hannah/TreeSign.png",
                    {1000.0/1920,125.0/1080,100.0/1920,80.0/1080},{0,0,1,1},.93,.20},
                {"Field house sign frame","PokemonBDSP/Autostory/Route208Hannah/HouseSign.png",
                    {1440.0/1920,110.0/1080,110.0/1920,110.0/1080},{0,0,1,1},.93,.20}
            };
            if (appearance){
                // Recorded dodges stand farther east, with the stairs clipped.
                // These agreeing signs guide acquisition only; the successful
                // departure reference above alone may confirm arrival.
                configs.erase(configs.begin());
                configs[0].path=appearance==865 ? "PokemonBDSP/Autostory/Route208Hannah/FieldSign865.png" :
                    appearance==930 ? "PokemonBDSP/Autostory/Route208Hannah/FieldSign930.png" : appearance==965 ? "PokemonBDSP/Autostory/Route208Hannah/FieldSign965.png" : "PokemonBDSP/Autostory/Route208Hannah/FieldSign978.png";
                configs[1].path=appearance==865 ? "PokemonBDSP/Autostory/Route208Hannah/HouseSign865.png" :
                    appearance==930 ? "PokemonBDSP/Autostory/Route208Hannah/HouseSign930.png" : appearance==965 ? "PokemonBDSP/Autostory/Route208Hannah/HouseSign965.png" : "PokemonBDSP/Autostory/Route208Hannah/HouseSign978.png";
            }
            for (auto& config:configs){
                const double w=config.target.width,h=config.target.height;
                config.target.width*=scale;config.target.height*=scale;
                config.target.x+=(w-config.target.width)/2;
                config.target.y+=(h-config.target.height)/2;
            }
            auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
            if (appearance){candidate.coarse=true;candidate.at_target=false;}
            double score=1;
            size_t accepted=0;
            for (const auto& landmark:candidate.landmarks) if (landmark.accepted) {score=std::min(score,landmark.score);accepted++;}
            if (!accepted) score=-1;
            if ((candidate.located && !best.located) ||
                (candidate.located && best.located && !candidate.coarse && best.coarse) ||
                (candidate.located==best.located && candidate.coarse==best.coarse && score>best_score)){
                best=std::move(candidate);best_score=score;
            }
        }
    }
    return best;
}
OverworldMatchResult match_hearthome_center_entrance(const ImageRGB32& frame){
    // Stop on the street, south of the trigger tile. The roof emblem and
    // orange awning must agree; a ball-shaped interior display is not enough.
    OverworldMatchResult best;
    double best_score=-1;
    for (const char* ball : {"PokemonBDSP/Autostory/HearthomeCenter/Ball.png",
        "PokemonBDSP/Autostory/HearthomeCenter/BallLive.png"}){
    for (double scale : {.90,.925,.95,.975,1.0,1.025,1.05,1.075,1.10}){
        const double w=112.0/1920*scale,h=108.0/1080*scale;
        const double aw=220.0/1920*scale,ah=50.0/1080*scale;
        auto candidate=match_landmarks(frame,{
            {"Hearthome Center roof ball",ball,
                {.5-w/2,170.0/1080-h/2,w,h},{0,0,1,.65},.95,.20},
            {"Hearthome Center orange awning","PokemonBDSP/Autostory/HearthomeCenter/Awning.png",
                {962.0/1920-aw/2,256.0/1080-ah/2,aw,ah},{0,0,1,.65},.95,.20}
        },2,1.0/128,POSITION_TOLERANCE_Y,true);
        double score=1;
        for (const auto& anchor:candidate.landmarks) score=std::min(score,anchor.score);
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)){
            best=std::move(candidate);best_score=score;
        }
    }
    }
    return best;
}
OverworldMatchResult match_hearthome_center_interior(const ImageRGB32& frame){
    // Three observed entry-camera angles. Perspective changes the machine
    // and register independently, so require a pair from the same real view.
    struct View { const char* machine; const char* desk; ImageFloatBox a,b; };
    const View views[]={
        {"InteriorMachine.png","InteriorRegister.png",{660.0/1920,160.0/1080,220.0/1920,185.0/1080},
            {1150.0/1920,155.0/1080,140.0/1920,175.0/1080}},
        {"EntryRightMachine.png","EntryRightRegister.png",{390.0/1920,160.0/1080,240.0/1920,185.0/1080},
            {905.0/1920,155.0/1080,130.0/1920,175.0/1080}},
        {"EntryLeftMachine.png","EntryLeftRegister.png",{1045.0/1920,160.0/1080,240.0/1920,185.0/1080},
            {1550.0/1920,220.0/1080,155.0/1920,110.0/1080}}
    };
    OverworldMatchResult best;
    double best_score=-1;
    for (const auto& view:views){
        const std::string machine="PokemonBDSP/Autostory/HearthomeCenter/"+std::string(view.machine);
        const std::string desk="PokemonBDSP/Autostory/HearthomeCenter/"+std::string(view.desk);
        for (double scale : {.90,.95,1.0,1.05,1.10}){
            auto a=view.a,b=view.b;
            a.x=.5+(a.x-.5)*scale;a.y=.5+(a.y-.5)*scale;a.width*=scale;a.height*=scale;
            b.x=.5+(b.x-.5)*scale;b.y=.5+(b.y-.5)*scale;b.width*=scale;b.height*=scale;
            auto candidate=match_landmarks(frame,{
                {"Center healing machine",machine.c_str(),a,{0,0,1,.8},.93,.20},
                {"Center register",desk.c_str(),b,{0,0,1,.8},.93,.20}
            },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
            double score=1;
            for (const auto& anchor:candidate.landmarks) score=std::min(score,anchor.score);
            if ((candidate.located && !best.located) ||
                (candidate.located==best.located && score>best_score)){
                best=std::move(candidate);best_score=score;
            }
        }
    }
    return best;
}
OverworldMatchResult match_hearthome_contest_fantina(const ImageRGB32& frame){
    // Fantina stands in front of the blue Contest Hall doors. Approach from
    // the south; the dress and the door above her must move together.
    OverworldMatchResult best;
    double best_score=-1;
    for (double scale : {.90,.95,1.0,1.05,1.10}){
        const double w=190.0/1920*scale,h=180.0/1080*scale;
        const double dw=220.0/1920*scale,dh=70.0/1080*scale;
        auto candidate=match_landmarks(frame,{
            {"Fantina pink dress","PokemonBDSP/Autostory/HearthomeContest/Fantina.png",
                {.5-w/2,410.0/1080-h/2,w,h},{0,.1,1,.8},.93,.15},
            {"Contest Hall blue door","PokemonBDSP/Autostory/HearthomeContest/BlueDoor.png",
                {930.0/1920-dw/2,285.0/1080-dh/2,dw,dh},{0,.05,1,.8},.93,.15}
        },2,1.0/128,1.0/54,true);
        double score=1;
        for (const auto& anchor:candidate.landmarks) score=std::min(score,anchor.score);
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)){
            best=std::move(candidate);best_score=score;
        }
    }
    return best;
}
OverworldMatchResult match_eterna_gym_entrance(const ImageRGB32& frame){
    // Recorded centered doorway pose. All three stationary landmarks must agree
    // before entering; two can guide acquisition without confirming arrival.
    OverworldMatchResult best;
    double best_score=-1;
    for (int appearance : {19,16}){
        for (double scale : {.90,.95,1.0,1.05,1.10}){
            std::vector<LandmarkConfig> configs={
                {"Left gym sign",appearance==19 ? "PokemonBDSP/Autostory/EternaGym/LeftSign19.png" : "PokemonBDSP/Autostory/EternaGym/LeftSign16.png",
                    {458.0/1920,358.0/1080,140.0/1920,150.0/1080},{0,0,.65,.85},.93,.20},
                {"Gym Pokeball",appearance==19 ? "PokemonBDSP/Autostory/EternaGym/Ball19.png" : "PokemonBDSP/Autostory/EternaGym/Ball16.png",
                    {880.0/1920,264.0/1080,150.0/1920,80.0/1080},{.15,0,.7,.70},.93,.20},
                {"Right bike post",appearance==19 ? "PokemonBDSP/Autostory/EternaGym/BikePost19.png" : "PokemonBDSP/Autostory/EternaGym/BikePost16.png",
                    {1468.0/1920,460.0/1080,130.0/1920,178.0/1080},{.35,.15,.65,.80},.93,.20}
            };
            for (auto& config:configs){
                const double w=config.target.width,h=config.target.height;
                config.target.width*=scale;config.target.height*=scale;
                config.target.x+=(w-config.target.width)/2;
                config.target.y+=(h-config.target.height)/2;
            }
            auto candidate=match_landmarks(frame,configs,3,1.0/128,POSITION_TOLERANCE_Y,true);
            if (!candidate.located){
                candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
                candidate.coarse=true;candidate.at_target=false;
            }
            double score=1;
            for(const auto& landmark:candidate.landmarks)if(landmark.accepted)score=std::min(score,landmark.score);
            if(candidate.located && (!best.located || (best.coarse && !candidate.coarse) ||
                (best.coarse==candidate.coarse && score>best_score))){best=candidate;best_score=score;}
            else if(best.landmarks.empty())best=candidate;
        }
    }
    if (best.located) return best;
    // The gym first appears at the far right of the approach street. Search
    // the whole frame for its left sign and emblem; the bike post may be offscreen.
    // These broad matches guide travel only. Arrival still needs all three above.
    double coarse_score=-1;
    for (int appearance : {680,19,333,303,306,339}){
        for (double scale : {.85,.90,1.0,1.10,1.15}){
            std::vector<LandmarkConfig> configs={
                {"Left gym sign",appearance==680 ? "PokemonBDSP/Autostory/EternaGym/LeftSignApproach.png" : appearance==19 ? "PokemonBDSP/Autostory/EternaGym/LeftSign19.png" :
                    appearance==333 ? "PokemonBDSP/Autostory/EternaGym/LeftSignWest.png" :
                    appearance==306 ? "PokemonBDSP/Autostory/EternaGym/LeftSignSouthShadow.png" :
                    appearance==339 ? "PokemonBDSP/Autostory/EternaGym/LeftSignWestShadow.png" : "PokemonBDSP/Autostory/EternaGym/LeftSignSouth.png",
                    {458.0/1920,358.0/1080,140.0/1920,150.0/1080},{0,0,1,.95},.93,.20},
                {"Gym Pokeball",appearance==680 ? "PokemonBDSP/Autostory/EternaGym/BallApproach.png" : appearance==19 ? "PokemonBDSP/Autostory/EternaGym/Ball19.png" : "PokemonBDSP/Autostory/EternaGym/BallWest.png",
                    {880.0/1920,264.0/1080,150.0/1920,80.0/1080},{0,0,1,.90},.93,.20}
            };
            if(appearance==680){
                configs[0].target.width=148.0/1920;configs[0].target.height=158.0/1080;
                configs[1].target.width=190.0/1920;
            }
            for(auto& config:configs){
                const double w=config.target.width,h=config.target.height;
                config.target.width*=scale;config.target.height*=scale;
                config.target.x+=(w-config.target.width)/2;config.target.y+=(h-config.target.height)/2;
            }
            auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
            const auto& sign=candidate.landmarks[0];const auto& ball=candidate.landmarks[1];
            double score=std::min(sign.score,ball.score);
            // Perspective changes their spacing while the gym is far right.
            // Both accepted anchors must still agree on the rightward direction
            // and vertical row before using that directional-only acquisition.
            if(!candidate.located && sign.accepted && ball.accepted &&
                sign.dx>.08 && ball.dx>.08 && std::abs(sign.dy-ball.dy)<=.04 &&
                std::min(sign.dx,ball.dx)>2*std::abs(sign.dx-ball.dx)){
                candidate.located=true;candidate.dx=(sign.dx+ball.dx)/2;candidate.dy=(sign.dy+ball.dy)/2;
            }
            if(!candidate.located){
                // One fully visible unique landmark is enough to guide the
                // approach when the other is clipped at the right edge.
                for(const auto& anchor:candidate.landmarks){
                    if(anchor.accepted && anchor.score>=.97 && anchor.margin>=.30 &&
                        anchor.dx>.08 && std::abs(anchor.dy)<=.22 && anchor.score>score){
                        candidate.located=true;candidate.dx=anchor.dx;candidate.dy=anchor.dy;score=anchor.score;
                    }
                }
            }
            candidate.coarse=true;candidate.at_target=false;
            if(candidate.located && score>coarse_score){best=candidate;coarse_score=score;}
        }
    }
    if(best.located)return best;
    // The yellow-roofed house opposite the gym remains visible when the
    // gym itself is clipped offscreen. Reuse paired landmark agreement to
    // recover the doorway row and guide east until gym anchors take over.
    // Its virtual target is off the left edge: these approach anchors can
    // never satisfy near-door moderate confidence or confirm gym arrival.
    double house_score=-1;
    for(int perspective:{720,734,708}){
    for(double scale:{.90,.95,1.0,1.05,1.10}){
        std::vector<LandmarkConfig> configs={
            {"Gym approach house door",perspective==708 ? "PokemonBDSP/Autostory/EternaGym/ApproachHouseDoorFarWest.png" : perspective==734 ? "PokemonBDSP/Autostory/EternaGym/ApproachHouseDoorWest.png" : "PokemonBDSP/Autostory/EternaGym/ApproachHouseDoor.png",
                {548.0/1920-.55,448.0/1080,128.0/1920,156.0/1080},{0,.15,.75,.70},.93,.20},
            {"Gym approach house window",perspective==708 ? "PokemonBDSP/Autostory/EternaGym/ApproachHouseWindowFarWest.png" : perspective==734 ? "PokemonBDSP/Autostory/EternaGym/ApproachHouseWindowWest.png" : "PokemonBDSP/Autostory/EternaGym/ApproachHouseWindow.png",
                {718.0/1920-.55,462.0/1080,132.0/1920,112.0/1080},{0,.15,.85,.70},.93,.20}
        };
        if(perspective==734){
            configs[0].target={338.0/1920-.55,450.0/1080,132.0/1920,150.0/1080};
            configs[1].target={496.0/1920-.55,464.0/1080,122.0/1920,102.0/1080};
        }
        if(perspective==708){
            configs[0].target={240.0/1920-.55,464.0/1080,150.0/1920,150.0/1080};
            configs[1].target={416.0/1920-.55,478.0/1080,118.0/1920,90.0/1080};
        }
        for(auto& config:configs){
            const double w=config.target.width,h=config.target.height;
            config.target.width*=scale;config.target.height*=scale;
            config.target.x+=(w-config.target.width)/2;config.target.y+=(h-config.target.height)/2;
        }
        auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
        candidate.coarse=true;candidate.at_target=false;
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if(candidate.located && candidate.dx>.08 && score>house_score){best=candidate;house_score=score;}
    }
    }
    return best;
}
double overworld_entry_confidence(const OverworldMatchResult& result,
    double tolerance_x, double tolerance_y, double minimum_margin){
    double confidence=0;
    for(size_t i=0;i<result.landmarks.size();i++){
        const auto& a=result.landmarks[i];
        if(a.margin<minimum_margin || std::abs(a.dx)>tolerance_x || std::abs(a.dy)>tolerance_y)continue;
        for(size_t j=i+1;j<result.landmarks.size();j++){
            const auto& b=result.landmarks[j];
            if(b.margin<minimum_margin || std::abs(b.dx)>tolerance_x || std::abs(b.dy)>tolerance_y)continue;
            if(std::abs(a.dx-b.dx)>AGREEMENT_X || std::abs(a.dy-b.dy)>AGREEMENT_Y)continue;
            confidence=std::max(confidence,std::min(a.score,b.score));
        }
    }
    return confidence;
}
OverworldMatchResult match_galactic_hq_jupiter(const ImageRGB32& frame){
    // The supplied reference is the interaction pose directly north of Jupiter.
    // Confirm the table-facing chair sides, adjacent table edges, and stairs.
    // Exclude the NPC-facing chair edges where Buneary can overlap the reference.
    const std::vector<LandmarkConfig> configs{
        {"LeftChairs","PokemonBDSP/Autostory/GalacticHQ/AboveLeftChairTableEdge.png",
            {300.0/1920,780.0/1080,156.0/1920,206.0/1080},{0,.40,.45,.60},.93,.20},
        {"RightChairs","PokemonBDSP/Autostory/GalacticHQ/AboveRightChairTableEdge.png",
            {1286.0/1920,780.0/1080,154.0/1920,206.0/1080},{.50,.40,.50,.60},.93,.20},
        {"StairBoundary","PokemonBDSP/Autostory/GalacticHQ/AboveStairs.png",
            {424.0/1920,310.0/1080,214.0/1920,198.0/1080},{0,0,1,.85},.93,.20}
    };
    auto result=match_landmarks(frame,configs,3,1.0/128,POSITION_TOLERANCE_Y,true);
    if (!result.located){
        // Two agreeing points guide the approach; all three still own arrival.
        auto coarse=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
        if (coarse.located){coarse.coarse=true;coarse.at_target=false;result=coarse;}
    }
    if (!result.located){
        // Existing recorded chair views can guide recovery from the old row.
        // Their coordinates still point to the new north-side destination;
        // only the three canonical above-Jupiter references confirm arrival.
        auto approach=std::vector<LandmarkConfig>{configs[0],configs[1]};
        approach[0].path="PokemonBDSP/Autostory/GalacticHQ/LeftChairs.png";
        approach[1].path="PokemonBDSP/Autostory/GalacticHQ/RightChairs.png";
        // Retain the original geometry for the old-row guide-only references.
        approach[0].target={334.0/1920,756.0/1080,156.0/1920,228.0/1080};
        approach[1].target={1238.0/1920,756.0/1080,164.0/1920,230.0/1080};
        for (auto& chair:approach){chair.threshold=.95;chair.margin=.25;}
        auto coarse=match_landmarks(frame,approach,2,1.0/128,POSITION_TOLERANCE_Y,true);
        if (coarse.located){
            result.located=true;result.coarse=true;result.at_target=false;
            result.dx=coarse.dx;result.dy=coarse.dy;
            result.landmarks[0]=coarse.landmarks[0];
            result.landmarks[1]=coarse.landmarks[1];
        }
    }
    return result;
}

OverworldMatchResult match_galactic_hq_cut(const ImageRGB32& frame){
    // The sign face and Center emblem identify the cuttable tree's column.
    // Exclude the HQ door, whose open/night appearance changed in the failed run.
    // Tight face/emblem crops avoid grass and moving cloud shadows on the roof.
    // The stronger emblem needs a corroborating sign, with a lower identity
    // gate for the smaller sign face. Both still must agree on the same pose.
    auto result=match_lighting_pair(frame,{
        {"HQ sign","PokemonBDSP/Autostory/GalacticHQ/CutSignStructure.png",
            {810.0/1920,500.0/1080,80.0/1920,58.0/1080},{0,.25,1,.45},.85,.10,LandmarkFeatures::LOCAL_CONTRAST},
        {"Center Pokeball","PokemonBDSP/Autostory/GalacticHQ/CutCenterBallStructure.png",
            {1048.0/1920,1004.0/1080,136.0/1920,66.0/1080},{0,.65,1,.35},.93,.20,LandmarkFeatures::BLUE_FRACTION}
    },"HQ sign","Center Pokeball",1.0/128);
    if (!result.located && result.landmarks.size()==2){
        const auto& sign=result.landmarks[0];const auto& ball=result.landmarks[1];
        // In the recorded far-west misses, the low fascia emblem shifts 1.31
        // screen pixels per sign pixel. Calibrate that acquisition observation
        // before applying the same position-agreement bound; never use it for arrival.
        const double calibrated_ball_dx=ball.dx/1.31;
        if (sign.accepted && ball.accepted && sign.dx*ball.dx>0 &&
            std::abs(sign.dx)>.03 && std::abs(sign.dx)<=.20 &&
            std::abs(sign.dx-calibrated_ball_dx)<=AGREEMENT_X &&
            std::abs(sign.dy-ball.dy)<=AGREEMENT_Y){
            result.located=true;result.coarse=true;result.at_target=false;
            result.dx=(sign.dx+calibrated_ball_dx)/2;
        }
    }
    // The recorded approach already clamps against the hedge. Correct only
    // its column; the watched Up/A loop owns the final north interaction.
    result.dy=0;
    result.at_target=result.located && !result.coarse && std::abs(result.dx)<=1.0/128;
    return result;
}

OverworldMatchResult match_eterna_cycle_shop_entrance(const ImageRGB32& frame){
    const std::vector<LandmarkConfig> configs{
        {"Shop sign","PokemonBDSP/Autostory/EternaBikeShop/eterna_cycle_shop_entrance-0.png",{555.0/1920,350.0/1080,100.0/1920,120.0/1080},{0,0.1,0.65,0.75},.93,.20},
        {"Shop roof braces","PokemonBDSP/Autostory/EternaBikeShop/ShopRoofBraces.png",{680.0/1920,145.0/1080,470.0/1920,36.0/1080},{0,0,.95,.75},.93,.20}
    };
    return match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
}

OverworldMatchResult match_eterna_cycle_shop_interior(const ImageRGB32& frame){
    // Entry mat view: fixed wall tools and register, clear of NPCs/Pokemon.
    // This identifies the room; it is not the counter interaction position.
    const std::vector<LandmarkConfig> configs{
        {"Shop wall tools","PokemonBDSP/Autostory/EternaBikeShop/InteriorWallTools.png",{920.0/1920,20.0/1080,350.0/1920,150.0/1080},{.15,0,.60,.55},.93,.20},
        {"Shop register","PokemonBDSP/Autostory/EternaBikeShop/InteriorRegister.png",{760.0/1920,160.0/1080,120.0/1920,145.0/1080},{.20,0,.55,.65},.93,.20}
    };
    auto result=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
    if (result.located) return result;
    const std::vector<LandmarkConfig> north_configs{
        {"Shop wall tools","PokemonBDSP/Autostory/EternaBikeShop/InteriorNorthWallTools.png",{928.0/1920,84.0/1080,350.0/1920,150.0/1080},{.15,0,.60,.55},.93,.20},
        {"Shop register","PokemonBDSP/Autostory/EternaBikeShop/InteriorNorthRegister.png",{754.0/1920,230.0/1080,120.0/1920,145.0/1080},{.20,0,.55,.65},.93,.20}
    };
    return match_landmarks(frame,north_configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
}

OverworldMatchResult match_eterna_cycle_shop_reward(const ImageRGB32& frame){
    // Reward returns the player outside on the bicycle, at a separate pose.
    // Reuse the stable roof braces; the nearby post changes appearance/shading.
    const std::vector<LandmarkConfig> configs{
        {"Shop sign","PokemonBDSP/Autostory/EternaBikeShop/reward-sign.png",{580.0/1920,330.0/1080,100.0/1920,120.0/1080},{0,.1,.65,.75},.93,.20},
        {"Shop roof braces","PokemonBDSP/Autostory/EternaBikeShop/ShopRoofBraces.png",{706.0/1920,128.0/1080,470.0/1920,36.0/1080},{0,0,.95,.75},.93,.20}
    };
    return match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
}

OverworldMatchResult match_eterna_bike_riding_card(const ImageRGB32& frame){
    // Fixed UI card: require both the blue illustration and B Close prompt.
    // Small search boxes tolerate capture offsets without matching other menus.
    const std::vector<LandmarkConfig> configs{
        {"Blue Bike illustration","PokemonBDSP/Autostory/EternaBikeShop/BikeRidingBlueFeature.png",
            {104.0/1920,314.0/1080,520.0/1920,450.0/1080},{.035,.26,.32,.48},.93,.20,LandmarkFeatures::BLUE_FRACTION},
        {"B Close prompt","PokemonBDSP/Autostory/EternaBikeShop/BikeRidingClosePrompt.png",
            {1570.0/1920,730.0/1080,160.0/1920,60.0/1080},{.79,.65,.14,.10},.93,.20}
    };
    return match_landmarks(frame,configs,2,1.0/64,1.0/54,true);
}

OverworldMatchResult match_eterna_cycle_shop_counter(const ImageRGB32& frame){
    const std::vector<LandmarkConfig> configs{
        {"Red frame","PokemonBDSP/Autostory/EternaBikeShop/eterna_cycle_shop_counter-0.png",{620.0/1920,65.0/1080,390.0/1920,240.0/1080},{0.1,0,0.65,0.55},.93,.20},
        {"Counter detail","PokemonBDSP/Autostory/EternaBikeShop/eterna_cycle_shop_counter-1.png",{1110.0/1920,486.0/1080,86.0/1920,38.0/1080},{0.3,0.15,0.65,0.65},.93,.20}
    };
    return match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
}

// Two stationary architectural features confirm the intended gate interior.
// The doorway and west window avoid the moving player, NPCs and localized text.
OverworldMatchResult match_cycling_road_south_gate_interior(const ImageRGB32& frame){
    return match_landmarks(frame,{
        {"Gate north doorway","PokemonBDSP/Autostory/CyclingRoadSouthGate/NorthDoorway.png",{630.0/1920,118.0/1080,220.0/1920,270.0/1080},{.15,0,.70,.55},.93,.20},
        {"Gate west window","PokemonBDSP/Autostory/CyclingRoadSouthGate/WestWindow.png",{302.0/1920,140.0/1080,157.0/1920,145.0/1080},{.05,0,.35,.55},.93,.20}
    },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y);
}
OverworldMatchResult match_cycling_road_bridge_approach(const ImageRGB32& frame){
    // Use the actual bridge entrance: cyan right rail with a broad metal deck
    // immediately to its left. Neither the Route 206 sign nor gate is involved.
    OverworldMatchResult best;
    if (!frame) return best;
    cv::Mat pixels((int)frame.height(),(int)frame.width(),CV_8UC4,
        const_cast<uint32_t*>(frame.data()),frame.bytes_per_row());
    cv::Mat small,bgr,hsv,cyan;
    cv::resize(pixels,small,cv::Size(960,540),0,0,cv::INTER_AREA);
    cv::cvtColor(small,bgr,cv::COLOR_BGRA2BGR);
    cv::cvtColor(bgr,hsv,cv::COLOR_BGR2HSV);
    // Hue/saturation retain the cyan rail under the recorded moving shadows.
    cv::inRange(hsv,cv::Scalar(78,95,55),cv::Scalar(105,255,255),cyan);
    cv::morphologyEx(cyan,cyan,cv::MORPH_CLOSE,cv::Mat::ones(3,3,CV_8U));
    const int w=hsv.cols,h=hsv.rows;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(cyan,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    double best_support=0;
    bool ambiguous=false;
    for (const auto& contour:contours){
        const auto rail=cv::boundingRect(contour);
        if (rail.x<w*.25 || rail.x>w*.85 || rail.height<h*.10) continue;
        // A left-hand rail has grass/cliff on this side and fails the deck test.
        const int left=std::max(0,rail.x-(int)(w*.22));
        const int right=rail.x-(int)(w*.02);
        if (right-left<w*.12) continue;
        const int begin=std::max(0,rail.y+(int)(h*.035));
        const int end=std::min(h-(int)(h*.05),rail.y+(int)(h*.24));
        const int sustained=std::max(3,(int)(h*.018));
        int deck_y=-1;
        double deck_support=0;
        for (int y=begin;y<end;y++){
            bool supported=true;
            double total=0;
            // Both halves must see the same broad deck edge, not a flower,
            // narrow painted line or one white patch beside a cyan object.
            for (int half=0;half<2;half++){
                int neutral=0;
                const int x0=left+(right-left)*half/2;
                const int x1=left+(right-left)*(half+1)/2;
                for (int row=y;row<y+sustained;row++)
                    for (int x=x0;x<x1;x++){
                        const auto p=hsv.at<cv::Vec3b>(row,x);
                        // Shadowed metal remains low-saturation; the grass edge
                        // and paired rail reject unrelated dark neutral panels.
                        neutral+=p[1]<=80 && p[2]>=60;
                    }
                const double fraction=(double)neutral/((x1-x0)*sustained);
                if (fraction<.72) supported=false;
                total+=fraction/2;
            }
            if (supported){deck_y=y;deck_support=total;break;}
        }
        if (deck_y<0 || deck_y<h*.15) continue;
        // The entrance is a grass-to-metal boundary. A road panel or sky
        // behind an unrelated cyan object cannot stand in for this edge.
        int grass=0;
        const int above_begin=std::max(0,deck_y-(int)(h*.05));
        const int above_end=deck_y-(int)(h*.02);
        if (above_end<=above_begin) continue;
        for (int row=above_begin;row<above_end;row++)
            for (int x=left;x<right;x++){
                const auto p=hsv.at<cv::Vec3b>(row,x);
                grass+=p[0]>=25 && p[0]<=85 && p[1]>=60 && p[2]>=30;
            }
        if ((double)grass/((right-left)*(above_end-above_begin))<.45) continue;
        // Measure the inner rail boundary at deck height, rather than using
        // the cliff wall or the rail cap's perspective-dependent bounding box.
        double inner=0,ink=0;size_t rows=0;
        const int measure_begin=std::max(deck_y+(int)(h*.015),(int)(h*.52));
        const int measure_end=std::min(h,measure_begin+(int)(h*.02));
        for (int y=measure_begin;y<measure_end;y++){
            for (int x=rail.x;x<std::min(w,rail.x+rail.width);x++){
                if (cyan.at<unsigned char>(y,x)) {
                    int last=x,count=0;
                    for (int edge=x;edge<std::min(w,rail.x+rail.width);edge++)
                        if (cyan.at<unsigned char>(y,edge)){last=edge;++count;}
                    const int width=last-x+1;
                    if (width>=w*.008 && width<=w*.055 && (double)count/width>=.80) {
                        inner+=x;ink+=(double)count/width;++rows;
                    }
                    break;
                }
            }
        }
        if (rows<3 || rows<(measure_end-measure_begin)*.75) continue;
        inner/=rows;
        OverworldMatchResult candidate;
        candidate.located=true;
        candidate.dx=inner/w-.55;
        candidate.dy=(double)deck_y/h-.36;
        // Park on the upper white platform beside the right rail. The user
        // demonstrated this stationary pose in the 12:33:35 recording; the
        // downhill section starts well beyond it. Measure the rail at wheel
        // height, not at its distant cap or the offset cliff wall.
        candidate.at_target=std::abs(candidate.dx)<=.012 &&
            std::abs(candidate.dy)<=1.0/54;
        OverworldLandmarkResult rail_anchor;
        rail_anchor.name="Bridge right rail inner edge";
        rail_anchor.accepted=true;rail_anchor.score=ink/rows;
        rail_anchor.dx=candidate.dx;rail_anchor.dy=candidate.dy;
        rail_anchor.search={.25,0,.68,1};
        rail_anchor.found={(double)inner/w,(double)rail.y/h,(double)rail.width/w,(double)rail.height/h};
        OverworldLandmarkResult deck_anchor;
        deck_anchor.name="Bridge deck leading edge";
        deck_anchor.accepted=true;deck_anchor.score=deck_support;
        deck_anchor.dx=candidate.dx;deck_anchor.dy=candidate.dy;
        deck_anchor.search={0,(double)begin/h,(double)right/w,(double)(end-begin)/h};
        deck_anchor.found={(double)left/w,(double)deck_y/h,(double)(right-left)/w,(double)sustained/h};
        candidate.landmarks={rail_anchor,deck_anchor};
        const double support=ink/rows+deck_support;
        if (best.located && (std::abs(best.dx-candidate.dx)>AGREEMENT_X ||
            std::abs(best.dy-candidate.dy)>AGREEMENT_Y)) ambiguous=true;
        if (support>best_support){best_support=support;best=std::move(candidate);}
    }
    if (ambiguous){best.located=false;best.at_target=false;}
    return best;
}


OverworldMatchResult match_galactic_hq_exterior(const ImageRGB32& frame){
    const std::vector<LandmarkConfig> configs{
        {"HQ sign","PokemonBDSP/Autostory/GalacticHQ/galactic_hq_exterior-0.png",{660.0/1920,615.0/1080,118.0/1920,124.0/1080},{0,0.2,0.65,0.8},.93,.20},
        {"HQ doorway lintel","PokemonBDSP/Autostory/GalacticHQ/galactic_hq_exterior-1.png",{874.0/1920,350.0/1080,172.0/1920,78.0/1080},{0.2,0,0.6,0.7},.93,.20}
    };
    return match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
}

OverworldMatchResult match_eterna_center_departure(const ImageRGB32& frame){
    // Center-exit pose in the recorded Center->Mart and bicycle-shop route.
    // Reuse the existing shadow/appearance-aware Center identification.
    auto result=match_eterna_center_entrance(frame);
    result.dy-=.070463;
    result.at_target=result.located && !result.coarse &&
        std::abs(result.dx)<=1.0/128 && std::abs(result.dy)<=POSITION_TOLERANCE_Y;
    return result;
}

OverworldMatchResult match_eterna_gym_statues(const ImageRGB32& frame){
    // Actual crops from screenshot-20261018-171535781091. Separate left/right
    // search regions prevent one identical statue from satisfying both votes.
    OverworldMatchResult best;double best_score=-1;
    if(!frame)return best;
    // Normalized correlation also matches almost-black fade frames. Require
    // visible statue regions before allowing those shapes to verify entry.
    const cv::Mat gray=gray_image(frame);
    const auto visible=[&](double x,double width){
        const cv::Rect box((int)(x*gray.cols),(int)(458.0/1080*gray.rows),
            (int)(width*gray.cols),(int)(350.0/1080*gray.rows));
        return cv::mean(gray(box))[0]>=60;
    };
    const bool statues_visible=visible(448.0/1920,146.0/1920)
        &&visible(1322.0/1920,154.0/1920);
    for(double scale:{.95,1.0,1.05}){
        std::vector<LandmarkConfig> configs={
            {"Left gym statue","PokemonBDSP/Autostory/EternaGym/LeftStatue.png",
                {448.0/1920,458.0/1080,146.0/1920,350.0/1080},{.12,.30,.27,.53},.93,.20},
            {"Right gym statue","PokemonBDSP/Autostory/EternaGym/RightStatue.png",
                {1322.0/1920,458.0/1080,154.0/1920,350.0/1080},{.60,.30,.28,.53},.93,.20}
        };
        for(auto& config:configs){
            const double w=config.target.width,h=config.target.height;
            config.target.width*=scale;config.target.height*=scale;
            config.target.x+=(w-config.target.width)/2;config.target.y+=(h-config.target.height)/2;
        }
        auto candidate=match_landmarks(frame,configs,2,.04,.06,true);
        // Interior verification requires both statues near the expected entry
        // positions, not merely anywhere in the room or during a fade.
        candidate.at_target=candidate.at_target&&statues_visible;
        candidate.located=candidate.at_target;
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if((candidate.located&&!best.located) || (candidate.located==best.located&&score>best_score)){
            best=candidate;best_score=score;
        }
    }
    return best;
}
OverworldMatchResult match_eterna_underground_house(const ImageRGB32& frame){
    // User's doorway pose: the Center ball and the same house sign, viewed
    // from the east. The sign is optional when a strong unique ball confirms
    // the doorway pose; cloud shadow on the sign must not block entry.
    OverworldMatchResult best;
    double best_score=-1;
    bool best_primary_only=false;
    for (double scale : {.90,.95,1.0,1.05,1.10}){
    std::vector<LandmarkConfig> configs{
        {"Center Pokeball","PokemonBDSP/Autostory/EternaUndergroundMan/HouseBall.png",
            {332.0/1920,224.0/1080,112.0/1920,104.0/1080},{0,0,.65,.7},.93,.20},
        {"House sign","PokemonBDSP/Autostory/EternaUndergroundMan/HouseSign.png",
            {1240.0/1920,396.0/1080,110.0/1920,106.0/1080},{.35,0,.65,.85},.93,.20}
    };
    for (auto& config:configs){
        const double w=config.target.width,h=config.target.height;
        config.target.width*=scale;config.target.height*=scale;
        config.target.x+=(w-config.target.width)/2;
        config.target.y+=(h-config.target.height)/2;
    }
    auto candidate=match_landmarks(frame,configs,2,1.0/128,POSITION_TOLERANCE_Y,true);
    double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
    if(candidate.located){
        if(!best.located || best_primary_only || score>best_score){best=candidate;best_score=score;best_primary_only=false;}
    }else if(!best.located || best_primary_only){
        // The optional door-side sign can change perspective or disappear in
        // cloud shadow. Preserve the high uniqueness margin on the primary ball.
        const auto& ball=candidate.landmarks[0];
        if(ball.accepted && ball.score>=.95 && ball.margin>=.30 && std::abs(ball.dx)<.20 && std::abs(ball.dy)<.20 &&
            (!best.located || ball.score>best_score)){
            best=candidate;best.located=true;best.coarse=false;
            best.dx=ball.dx;best.dy=ball.dy;
            best.at_target=std::abs(best.dx)<=1.0/128 && std::abs(best.dy)<=POSITION_TOLERANCE_Y;
            best_score=ball.score;best_primary_only=true;
        }else if(best.landmarks.empty())best=candidate;
    }
    }
    return best;
}
OverworldMatchResult match_eterna_underground_man(const ImageRGB32& frame){
    // Reuse fixed furniture, excluding people and the animated TV picture.
    // The right-window search excludes the identical window above the stool.
    const std::vector<LandmarkConfig> configs{
        {"Stool","PokemonBDSP/Autostory/EternaUndergroundMan/Stool.png",
            {380.0/1920,610.0/1080,86.0/1920,70.0/1080},{0,.3,.65,.5},.93,.20},
        {"Right tree top","PokemonBDSP/Autostory/EternaUndergroundMan/TreeTop.png",
            {1372.0/1920,320.0/1080,102.0/1920,96.0/1080},{.5,0,.4,.7},.93,.20},
        {"Wall sign","PokemonBDSP/Autostory/EternaUndergroundMan/WallSign.png",
            {642.0/1920,354.0/1080,114.0/1920,70.0/1080},{.2,0,.5,.6},.93,.20},
        {"Window beside TV","PokemonBDSP/Autostory/EternaUndergroundMan/RightWindow.png",
            {906.0/1920,260.0/1080,162.0/1920,160.0/1080},{.35,0,.55,.55},.93,.20}
    };
    auto result=match_landmarks(frame,configs,4,1.0/128,POSITION_TOLERANCE_Y,true);
    if(!result.located){
        // Three agreeing furniture anchors identify the room during entry and
        // guide acquisition when a fourth is clipped. Never authorize talking.
        auto coarse=match_landmarks(frame,configs,3,1.0/128,POSITION_TOLERANCE_Y,true);
        if(coarse.located){coarse.coarse=true;coarse.at_target=false;return coarse;}
    }
    return result;
}
OverworldMatchResult match_jubilife_street_approach(const ImageRGB32& frame){
    // The lighting-run pose was 1/108 screen height south of the original
    // screenshot. Targets preserve the original destination height.
    // Capture-derived references cover lamp, daylight, and moving tree shadows.
    return match_lighting_pair(frame,{
        {"BuildingCorner","PokemonBDSP/Autostory/JubilifeStreet/BuildingCornerHour3Frame4.png",
            {0.588541666667,0.351851851852,0.088541666667,0.259259259259},{0,0,1,1},.93,.08},
        {"BuildingCorner","PokemonBDSP/Autostory/JubilifeStreet/BuildingCornerHour14Frame2.png",
            {0.588541666667,0.351851851852,0.088541666667,0.259259259259},{0,0,1,1},.93,.08},
        {"BuildingCorner","PokemonBDSP/Autostory/JubilifeStreet/BuildingCornerHour17Frame4.png",
            {0.588541666667,0.351851851852,0.088541666667,0.259259259259},{0,0,1,1},.93,.08},
        {"BuildingBase","PokemonBDSP/Autostory/JubilifeStreet/BuildingBaseHour15Frame2.png",
            {0.812500000000,0.388888888889,0.078125000000,0.203703703704},{0,0,1,1},.93,.08},
        {"BuildingBase","PokemonBDSP/Autostory/JubilifeStreet/BuildingBaseHour17Frame5.png",
            {0.812500000000,0.388888888889,0.078125000000,0.203703703704},{0,0,1,1},.93,.08},
        {"BuildingBase","PokemonBDSP/Autostory/JubilifeStreet/BuildingBaseHour20Frame5.png",
            {0.812500000000,0.388888888889,0.078125000000,0.203703703704},{0,0,1,1},.93,.08}
    },"BuildingCorner","BuildingBase",POSITION_TOLERANCE_X,true);
}
OverworldMatchResult match_jubilife_plaza_approach(const ImageRGB32& frame){
    // Capture-derived references cover lamp, daylight, and moving tree shadows.
    auto result=match_lighting_pair(frame,{
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour2Frame2.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour11Frame1.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour11Frame2.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour11Frame5.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour16Frame5.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"RoofEdge","PokemonBDSP/Autostory/JubilifePlaza/RoofEdgeHour23Frame2.png",
            {0.614583333333,0.203703703704,0.299479166667,0.111111111111},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour3Frame1.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour5Frame4.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour11Frame3.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour14Frame1.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour19Frame1.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08},
        {"Fence","PokemonBDSP/Autostory/JubilifePlaza/FenceHour21Frame4.png",
            {0.044270833333,0.328703703704,0.169270833333,0.106481481481},{0,0,1,1},.93,.08}
    // Small screen-relative quantization allowance avoids bouncing across the
    // plaza target when one movement step spans both sides of the tolerance.
    },"RoofEdge","Fence",POSITION_TOLERANCE_X,false,1.0/32);
    if (!result.located && result.landmarks.size()==2){
        const auto& fence=result.landmarks[1];
        // A moving roof shadow must not trigger a blind lateral sweep when the
        // distinctive fence still gives a strong, nearby position estimate.
        // Fence-only guidance can reacquire the roof; it cannot confirm arrival.
        if (fence.accepted && fence.score>=.95 && fence.margin>=.30 &&
            std::abs(fence.dx)<=.20 && std::abs(fence.dy)<=.20){
            result.located=true;
            result.coarse=true;
            result.at_target=false;
            result.dx=fence.dx;
            result.dy=fence.dy;
        }
    }
    return result;
}
OverworldMatchResult match_route204_sign(const ImageRGB32& frame){
    // One vote per physical landmark. Lighting variants avoid treating the
    // moving tree shadow over the fence as a missing route destination.
    const ImageFloatBox sign_target{0.468750000000,0.376851851852,0.072916666667,0.052777777778};
    const ImageFloatBox fence_target{0.598958333333,0.421296296296,0.041666666667,0.152777777778};
    std::vector<LandmarkConfig> variants{
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/Sign.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCorner.png",fence_target,{0,0,1,1},.93,.10},
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/SignHour0Frame3.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour0Frame3.png",fence_target,{0,0,1,1},.93,.10},
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/SignHour8Frame3.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour8Frame3.png",fence_target,{0,0,1,1},.93,.10},
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/SignHour13Frame3.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour13Frame3.png",fence_target,{0,0,1,1},.93,.10},
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/SignHour17Frame3.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour17Frame3.png",fence_target,{0,0,1,1},.93,.10},
        {"Route sign","PokemonBDSP/Autostory/Route204Sign/SignHour2Frame1.png",sign_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour2Frame1.png",fence_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerHour10Frame3.png",fence_target,{0,0,1,1},.93,.10},
        {"Fence corner","PokemonBDSP/Autostory/Route204Sign/FenceCornerRecordedCloud.png",fence_target,{0,0,1,1},.93,.10},
    };
    auto result=match_lighting_pair(frame,variants,"Route sign","Fence corner");
    if (!result.located && result.landmarks.size()==2){
        const auto& sign=result.landmarks[0];
        // A distinctive high-confidence sign can guide bounded acquisition.
        // It cannot confirm the endpoint without the fence agreeing as well.
        if (sign.accepted && sign.score>=.95 && sign.margin>=.30 &&
            std::abs(sign.dx)<=.20 && std::abs(sign.dy)<=.25){
            result.located=true;
            result.coarse=true;
            result.at_target=false;
            result.dx=sign.dx;
            result.dy=sign.dy;
        }
    }
    return result;
}
static OverworldMatchResult match_route205_marker(const ImageRGB32& frame, double cx, double cy){
    OverworldMatchResult best;
    // Native sign faces at the two route positions. The post, moving NPCs,
    // grass and water are excluded from the acquisition references.
    for (int variant : {0,1,2,3,4,5,6}) {
        for (double scale : {.9,1.0,1.1,1.2}) {
            const double w=(variant==6?188.0:variant==5?184.0:variant==4?220.0:variant==3?211.0:variant==2?208.0:variant==1?206.0:205.0)/1920*scale;
            const double h=(variant==6?74.0:variant>=3?72.0:variant?80.0:82.0)/1080*scale;
            auto candidate=match_landmarks(frame,{{"Route 205 marker",
                variant==6?"PokemonBDSP/Autostory/Route205/BridgeSignMidCloud.png":
                variant==5?"PokemonBDSP/Autostory/Route205/BridgeSignCloud.png":
                variant==4?"PokemonBDSP/Autostory/Route205/BridgeSignLow.png":
                variant==3?"PokemonBDSP/Autostory/Route205/BridgeSignLive.png":
                variant==2?"PokemonBDSP/Autostory/Route205/NorthSignRecorded.png":
                variant==1?"PokemonBDSP/Autostory/Route205/NorthSign.png":
                           "PokemonBDSP/Autostory/Route205/BridgeSign.png",
                {cx-w/2,cy-h/2,w,h},{0,0,1,1},.93,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
            if (best.landmarks.empty() || (candidate.located && !best.located) ||
                (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
        }
    }
    return best;
}
OverworldMatchResult match_route205_zackary(const ImageRGB32& frame){
    OverworldMatchResult best;
    double best_score=-1;
    for (int view : {0,1,2,3}) {
      for (double scale : {.9,1.0,1.1}) {
        const double sw=91.0/1920*scale,sh=65.0/1080*scale;
        // Include the fixed cliff edge beside both pipes: the repeated blue caps
        // alone produce a second strong peak in cloud shadow.
        const double cw=300.0/1920*scale,ch=300.0/1080*scale;
        auto candidate=match_landmarks(frame,{
            {"Zackary bridge sign",view==2?"PokemonBDSP/Autostory/Route205/PatrolSignCloud.png":
                view>=1?"PokemonBDSP/Autostory/Route205/PatrolSignLive.png":"PokemonBDSP/Autostory/Route205/PatrolSign.png",
                {959.5/1920-sw/2,67.5/1080-sh/2,sw,sh},{0,0,1,1},.93,.20},
            {"Zackary twin chimneys",view==3?"PokemonBDSP/Autostory/Route205/PatrolChimneysPartialCloud.png":
                view==2?"PokemonBDSP/Autostory/Route205/PatrolChimneysCloud.png":
                view==1?"PokemonBDSP/Autostory/Route205/PatrolChimneysLive.png":"PokemonBDSP/Autostory/Route205/PatrolChimneys.png",
                {1210.0/1920-cw/2,290.0/1080-ch/2,cw,ch},{0,0,1,1},.93,.20}
        },2,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>AGREEMENT_X || std::abs(best.dy-candidate.dy)>AGREEMENT_Y)) {
            best.located=false;best.at_target=false;return best;
        }
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)) {best=std::move(candidate);best_score=score;}
    }
    }
    return best;
}
OverworldMatchResult match_route205_kelsey(const ImageRGB32& frame){
    OverworldMatchResult best;
    // This item is retained on the route. Restrict acquisition to the left of
    // the player, excluding the red cap and the northern bridge's item ball.
    for (double scale : {.9,1.0,1.1}) {
        const double w=56.0/1920*scale,h=53.0/1080*scale;
        auto candidate=match_landmarks(frame,{{"Kelsey west item ball",
            "PokemonBDSP/Autostory/Route205/KelseyItem.png",
            {280.0/1920-w/2,574.5/1080-h/2,w,h},{0,.2,.48,.65},.93,.20}},
            1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
        if (best.landmarks.empty() || (candidate.located && !best.located) ||
            (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=std::move(candidate);
    }
    return best;
}
OverworldMatchResult match_route205_bank_tree(const ImageRGB32& frame){
    // Use the tree crown above the player's reach, plus the stationary child.
    // Recorded cloud-edge views retain both anchor gates and agreement.
    // Trunk crops can include the player's cap when he stands under the tree.
    OverworldMatchResult best;
    double best_score=-1;
    for (int view : {0,1,2,3,4,5,6,7,8}) {
      for (double scale : {.9,1.0,1.1}) {
        const double tw=140.0/1920*scale,th=130.0/1080*scale;
        const double nw=120.0/1920*scale,nh=170.0/1080*scale;
        auto candidate=match_landmarks(frame,{
            {"Bank tree crown",view==8?"PokemonBDSP/Autostory/Route205/BankTreeCrownWest.png":
                view==7?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud333.png":
                view==6?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud76.png":
                view==5?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud336.png":
                                   view==4?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud328.png":
                                   view==3?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud80.png":
                                   view==2?"PokemonBDSP/Autostory/Route205/BankTreeCrownCloud72.png":
                                   view==1?"PokemonBDSP/Autostory/Route205/BankTreeCrownLive.png":
                                   "PokemonBDSP/Autostory/Route205/BankTreeCrown.png",
                {770.0/1920-tw/2,235.0/1080-th/2,tw,th},{0,0,1,1},.93,.20},
            {"Bank blue-hat child",view==8?"PokemonBDSP/Autostory/Route205/BankBlueHatWest.png":
                view==7?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud333.png":
                view==6?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud76.png":
                view==5?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud336.png":
                                      view==4?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud328.png":
                                      view==3?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud80.png":
                                      view==2?"PokemonBDSP/Autostory/Route205/BankBlueHatCloud72.png":
                                      view==1?"PokemonBDSP/Autostory/Route205/BankBlueHatLive.png":
                                      "PokemonBDSP/Autostory/Route205/BankBlueHat.png",
                {700.0/1920-nw/2,525.0/1080-nh/2,nw,nh},{0,0,1,1},.93,.20}
        },2,1.0/96,POSITION_TOLERANCE_Y,true);
        const double score=std::min(candidate.landmarks[0].score,candidate.landmarks[1].score);
        if (best.located && candidate.located &&
            (std::abs(best.dx-candidate.dx)>AGREEMENT_X || std::abs(best.dy-candidate.dy)>AGREEMENT_Y)) {
            best.located=false;best.at_target=false;return best;
        }
        if ((candidate.located && !best.located) ||
            (candidate.located==best.located && score>best_score)) {
            best=std::move(candidate);best_score=score;
        }
      }
    }
    return best;
}
OverworldMatchResult match_route205_bridge(const ImageRGB32& frame){
    return match_route205_marker(frame,722.5/1920,465.0/1080);
}
OverworldMatchResult match_route205_forest_entrance(const ImageRGB32& frame){
    return match_route205_marker(frame,1245.0/1920,508.0/1080);
}
OverworldMatchResult match_windworks_interior_grunt(const ImageRGB32& frame){
    OverworldMatchResult result;
    cv::Mat pixels((int)frame.height(),(int)frame.width(),CV_8UC4,
        (void*)frame.data(),frame.bytes_per_row());
    cv::Mat small,bgr,hsv;
    cv::resize(pixels,small,cv::Size(960,540),0,0,cv::INTER_AREA);
    cv::cvtColor(small,bgr,cv::COLOR_BGRA2BGR);
    cv::cvtColor(bgr,hsv,cv::COLOR_BGR2HSV);
    // Grey indoor tiles distinguish this scene from the outdoor grunt, battle
    // screens and fades. Pale turquoise partitions have too little saturation
    // to qualify as the grunt's hair.
    cv::Mat floor;
    cv::inRange(hsv,cv::Scalar(0,0,100),cv::Scalar(179,65,255),floor);
    const cv::Rect room(288,81,624,405);
    if ((double)cv::countNonZero(floor(room))/room.area()<.30) return result;
    auto head=[&](cv::Scalar low,cv::Scalar high,bool player,cv::Point2d& center) {
        cv::Mat mask,labels,stats,centroids;
        cv::inRange(hsv,low,high,mask);
        const int count=cv::connectedComponentsWithStats(mask,labels,stats,centroids);
        int best=-1,best_area=0,runner_area=0;
        for (int i=1;i<count;i++) {
            int area=stats.at<int>(i,cv::CC_STAT_AREA);
            int w=stats.at<int>(i,cv::CC_STAT_WIDTH),h=stats.at<int>(i,cv::CC_STAT_HEIGHT);
            double x=centroids.at<double>(i,0),y=centroids.at<double>(i,1);
            if (area<80 || area>4000 || w<12 || w>110 || h<8 || h>90 ||
                (double)area/(w*h)<.35 || x<288 || x>(player?672:864) ||
                y<(player?135:15) || y>459) continue;
            if (area>best_area) {runner_area=best_area;best_area=area;best=i;}
            else runner_area=std::max(runner_area,area);
        }
        if (best<0 || runner_area>best_area*.70) return false;
        center={centroids.at<double>(best,0),centroids.at<double>(best,1)};
        return true;
    };
    cv::Point2d player,grunt;
    if (!head({0,120,120},{22,255,255},true,player) ||
        !head({75,100,120},{105,255,255},false,grunt)) return result;
    if (std::abs(player.x-grunt.x)>288 || std::abs(player.y-grunt.y)>350) return result;
    result.located=true;
    result.dx=(grunt.x-player.x)/960;
    result.dy=(grunt.y-player.y)/540;
    return result;
}
OverworldMatchResult match_windworks_sign_approach(const ImageRGB32& frame){
    // Sign beside the open lane west of the doorway turbine. Keep the player
    // east of the sign/grass before the blind building-wall approach.
    OverworldMatchResult best;
    // The sign grows as it approaches the bottom of the camera view. Scale
    // acquisition templates around the same target center, keeping all gates.
    for (int view : {0,1,2}) {
      for (double scale : {1.0,1.1,1.2,1.3}) {
        const double w=(view==2?124.0:view==1?144.0:121.0)/1920*scale;
        const double h=(view==1?140.0:122.0)/1080*scale;
        auto candidate=match_landmarks(frame,{{"Windworks approach sign",
            view==2?"PokemonBDSP/Autostory/WindworksApproach/SignNight.png":
            view==1?"PokemonBDSP/Autostory/WindworksApproach/SignLow.png":
                    "PokemonBDSP/Autostory/WindworksApproach/Sign.png",
            {836.5/1920-w/2,515.0/1080-h/2,w,h},
            {0,0,1,1},.93,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
        if (best.landmarks.empty() || (candidate.located && !best.located) ||
            (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
    }
    }
    // Face-only references cover the high/mid camera perspectives without
    // relying on the moving grass that can cover the sign's lower border.
    for (bool high_view : {false,true}) {
        for (double scale : {.9,1.0,1.1}) {
            const double w=(high_view?86.0:106.0)/1920*scale;
            const double h=(high_view?72.0:94.0)/1080*scale;
            auto candidate=match_landmarks(frame,{{"Windworks approach sign face",
                high_view?"PokemonBDSP/Autostory/WindworksApproach/SignHighFace.png":
                          "PokemonBDSP/Autostory/WindworksApproach/SignMidFace.png",
                {838.0/1920-w/2,516.0/1080-h/2,w,h},
                {0,0,1,1},.93,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
            if ((candidate.located && !best.located) ||
                (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
        }
    }
    return best;
}
OverworldMatchResult match_route204_floaroma_turn(const ImageRGB32& frame){
    OverworldMatchResult best;
    // The northern 204 marker is wider in this camera view. Match its face,
    // placing it above the player so the westward turn clears its post and
    // the flower-bed fence. Align horizontally with the sign before the
    // subsequent westward route, rather than staying in the eastern lane.
    for (double scale_x : {1.0,1.05,1.10,1.15,1.20}) {
        for (double scale_y : {.90,1.0,1.10}) {
            const double w=140.0/1920*scale_x, h=57.0/1080*scale_y;
            auto candidate=match_landmarks(frame,{{"Northern Route 204 sign",
                "PokemonBDSP/Autostory/Route204Sign/SignHour13Frame3.png",
                {.5-w/2,420.0/1080-h/2,w,h},{0,0,1,1},.93,.20}},1,POSITION_TOLERANCE_X,POSITION_TOLERANCE_Y,true);
            if (best.landmarks.empty() || (candidate.located && !best.located) ||
                (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
        }
    }
    return best;
}
OverworldMatchResult match_route204_north_approach(const ImageRGB32& frame){
    auto result=match_route204_sign(frame);
    // The pre-battle reference stands below the sign. After Tyler, place
    // the player one body-width west of it before starting north.
    constexpr double west_offset=.04;
    for (auto& anchor:result.landmarks){anchor.dx-=west_offset;}
    result.dx-=west_offset;
    if (!result.located && !result.landmarks.empty()){
        const auto& sign=result.landmarks[0];
        if (sign.accepted && sign.score>=.93 && sign.margin>=.30 &&
            std::abs(sign.dx)<=.45 && std::abs(sign.dy)<=.25){
            result.located=true;result.coarse=true;result.dx=sign.dx;result.dy=sign.dy;
        }
    }
    result.at_target=result.located && !result.coarse;
    for (const auto& anchor:result.landmarks)
        if (!anchor.accepted || std::abs(anchor.dx)>1.0/128 ||
            std::abs(anchor.dy)>POSITION_TOLERANCE_Y) result.at_target=false;
    return result;
}
OverworldMatchResult match_floaroma_house_entrance(const ImageRGB32& frame){
    // Fixed facade details above/aside the player avoid door/player occlusion.
    // Both must agree on the pose recorded just outside the central house.
    return match_landmarks(frame,{
        {"House window","PokemonBDSP/Autostory/FloaromaHouse/Window.png",
            {1060.0/1920,370.0/1080,128.0/1920,106.0/1080},{0,0,1,1},.93,.10},
        {"House gable","PokemonBDSP/Autostory/FloaromaHouse/Gable.png",
            {796.0/1920,196.0/1080,460.0/1920,154.0/1080},{0,0,1,1},.93,.10}
    },2,1.0/64,1.0/54,true);
}
OverworldMatchResult match_floaroma_center_entrance(const ImageRGB32& frame){
    OverworldMatchResult best;
    // Perspective changes the emblem size as we approach the doorway gap.
    for (double scale : {.90,.95,1.0,1.05,1.10}){
        const double w=105.0/1920*scale, h=86.0/1080*scale;
        auto candidate=match_landmarks(frame,{{"Center emblem",
            "PokemonBDSP/Autostory/FloaromaCenter/Emblem.png",
            {.5-w/2,135.0/1080-h/2,w,h},{0,0,1,1},.93,.20}},1,1.0/64,1.0/36,true);
        if (best.landmarks.empty() || (candidate.located && !best.located) ||
            (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
    }
    // At the west wall the camera makes the emblem nearly circular, unlike
    // the vertically compressed doorway reference. Search that recorded
    // perspective without lowering the score or uniqueness requirements.
    if (!best.located) {
        for (double scale_x : {1.0,1.05,1.10,1.15,1.20}) {
            for (double scale_y : {1.15,1.25,1.35,1.45}) {
                const double w=105.0/1920*scale_x, h=86.0/1080*scale_y;
                auto candidate=match_landmarks(frame,{{"Center emblem west perspective",
                    "PokemonBDSP/Autostory/FloaromaCenter/Emblem.png",
                    {.5-w/2,135.0/1080-h/2,w,h},{0,0,1,1},.93,.20}},1,1.0/64,1.0/36,true);
                candidate.coarse=true;
                candidate.at_target=false;
                if ((candidate.located && !best.located) ||
                    (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
            }
        }
    }
    // A lower-face reference excludes the tester's burned-in overlay line
    // and covers the doorway-facing perspective, retaining score/margin gates.
    auto lower=match_landmarks(frame,{{"Center emblem lower face",
        "PokemonBDSP/Autostory/FloaromaCenter/EmblemLower.png",
        {.5-52.0/1920,114.0/1080,104.0/1920,65.0/1080},{0,0,1,1},.93,.20}},
        1,1.0/64,1.0/36,true);
    if ((lower.located && (!best.located || best.coarse)) || (lower.located==best.located &&
        lower.landmarks[0].score>best.landmarks[0].score)) best=lower;
    return best;
}
OverworldMatchResult match_route204_north_stairs(const ImageRGB32& frame){
    OverworldMatchResult best;
    // Fixed rails/steps, with sunny and cloud-covered perspectives.
    for (const char* path:{"PokemonBDSP/Autostory/Route204NorthStairs/Stairs.png",
        "PokemonBDSP/Autostory/Route204NorthStairs/StairsCloud.png"}){
        // Camera pitch changes height and width by different amounts.
        for (double scale_x:{.90,1.0,1.10}) for (double scale_y:{.80,.90,1.0,1.10}){
            const double w=298.0/1920*scale_x,h=282.0/1080*scale_y;
            auto candidate=match_landmarks(frame,{{"North route stairs",path,
                {.5-w/2,650.0/1080,w,h},{0,0,1,1},.93,.10}},1,1.0/64,1.0/54,true);
            if (best.landmarks.empty() || (candidate.located && !best.located) ||
                (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
        }
    }
    if (!best.located){
        // The lower steps distort as the camera moves north. The rail tops,
        // grass edge and first steps retain a stable physical upper edge.
        for (const auto& config:std::vector<LandmarkConfig>{
            {"North route stairs top","PokemonBDSP/Autostory/Route204NorthStairs/Top.png",
                {.5-149.0/1920,640.0/1080,298.0/1920,70.0/1080},{0,0,1,1},.93,.10},
            {"North route stairs top","PokemonBDSP/Autostory/Route204NorthStairs/TopRaised.png",
                {.5-157.0/1920,640.0/1080,314.0/1920,70.0/1080},{0,0,1,1},.93,.10}
        }) for (double scale_x:{.95,1.0,1.05}) for (double scale_y:{.90,1.0,1.10}){
            auto variant=config;
            variant.target.width*=scale_x;variant.target.height*=scale_y;
            variant.target.x=.5-variant.target.width/2;
            auto candidate=match_landmarks(frame,{variant},1,1.0/64,1.0/54,true);
            if ((candidate.located && !best.located) ||
                (candidate.located==best.located && candidate.landmarks[0].score>best.landmarks[0].score)) best=candidate;
        }
    }
    return best;
}
OverworldMatchResult match_teighler_alcove_corner(const ImageRGB32& frame){
    auto result=match_route204_north_stairs(frame);
    // Reuse the fixed stairs only as an anchor. The destination is the
    // bottom-left of Teighler's alcove, east of the stairs, not their center.
    // Geometry comes from the recorded 1920x1080 corner reference.
    constexpr double shift_x=393.0/1920, shift_y=182.0/1080;
    result.dx+=shift_x;result.dy+=shift_y;
    for (auto& anchor:result.landmarks){anchor.dx+=shift_x;anchor.dy+=shift_y;}
    result.at_target=result.located && std::abs(result.dx)<=1.0/64 &&
        std::abs(result.dy)<=1.0/54;
    return result;
}
OverworldMatchResult match_ravaged_path_south_entrance(const ImageRGB32& frame){
    const ImageFloatBox mouth{860.0/1920,220.0/1080,200.0/1920,155.0/1080};
    const ImageFloatBox sign{810.0/1920,380.0/1080,100.0/1920,110.0/1080};
    std::vector<LandmarkConfig> variants;
    // A different approach height changes perspective. Compare a small range
    // of sizes, keeping each physical anchor's reference center fixed.
    for (double scale : {.90,1.0,1.10}){
        for (const auto& config : std::vector<LandmarkConfig>{
            {"Cave mouth","PokemonBDSP/Autostory/RavagedPathSouthEntrance/Mouth.png",mouth,{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/Sign.png",sign,{0,0,1,1},.93,.10},
            {"Cave mouth","PokemonBDSP/Autostory/RavagedPathSouthEntrance/MouthShadow.png",mouth,{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/SignShadow.png",sign,{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/SignPartialShadow.png",sign,{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/SignFace.png",{828.0/1920,400.0/1080,64.0/1920,62.0/1080},{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/SignFaceLowApproach.png",{828.0/1920,400.0/1080,64.0/1920,62.0/1080},{0,0,1,1},.93,.10},
            {"Entrance sign","PokemonBDSP/Autostory/RavagedPathSouthEntrance/SignFaceCloud.png",{828.0/1920,400.0/1080,64.0/1920,62.0/1080},{0,0,1,1},.93,.10}
        }){
            auto variant=config;
            variant.target.width*=scale;
            variant.target.height*=scale;
            variant.target.x+=(config.target.width-variant.target.width)/2;
            variant.target.y+=(config.target.height-variant.target.height)/2;
            variants.push_back(variant);
        }
    }
    auto result=match_lighting_pair(frame,variants,"Cave mouth","Entrance sign",1.0/64,true);
    if (!result.located && result.landmarks.size()==2){
        for (const auto& landmark : result.landmarks){
            if (landmark.accepted && landmark.score>=.95 && landmark.margin>=.20 &&
                std::abs(landmark.dx)<=.35 && std::abs(landmark.dy)<=.40){
                result.located=true;result.coarse=true;result.at_target=false;
                result.dx=landmark.dx;result.dy=landmark.dy;
                break;
            }
        }
    }
    // The moving blue-hatted boy is deliberately absent from the references.
    // The lass is optional evidence for the southern movement boundary, never
    // a replacement for the fixed mouth/sign arrival anchors.
    auto boundary=match_landmarks(frame,{{"Lass south boundary",
        "PokemonBDSP/Autostory/RavagedPathSouthEntrance/Lass.png",
        {910.0/1920,788.0/1080,100.0/1920,142.0/1080},{0,0,1,1},.88,.08
    }},1,1.0/64,1.0/36,true);
    if (!boundary.landmarks.empty()) result.landmarks.push_back(boundary.landmarks[0]);
    return result;
}
const std::vector<OverworldMatcher>& overworld_matchers(){    static const std::vector<OverworldMatcher> matchers={
        {"route204-north-approach","Route 204 left of sign after Tyler",match_route204_north_approach},
        {"teighler-alcove-corner","Teighler alcove bottom-left corner",match_teighler_alcove_corner},
        {"route204-north-stairs","Route 204 north stairs junction",match_route204_north_stairs},
        {"floaroma-center-entrance","Floaroma Pokemon Center entrance",match_floaroma_center_entrance},
        {"ravaged-path-south-entrance","Ravaged Path south entrance",match_ravaged_path_south_entrance},
        {"route204-sign","Route 204 sign before the next trainer",match_route204_sign},
        {"windworks-sign","Valley Windworks west of the door grunt",match_windworks_sign_approach},
        {"route205-bridge","Route 205 south of the bridge",match_route205_bridge},
        {"route205-bank-tree","Route 205 north bank beside the golden tree",match_route205_bank_tree},
        {"route205-zackary","Route 205 Zackary patrol intercept",match_route205_zackary},
        {"route205-kelsey","Route 205 Kelsey patrol intercept",match_route205_kelsey},
        {"route205-forest","Route 205 outside Eterna Forest",match_route205_forest_entrance},
        {"jubilife-street-approach","Jubilife street before Galactic plaza",match_jubilife_street_approach},
        {"jubilife-plaza-approach","Jubilife Galactic plaza approach",match_jubilife_plaza_approach},
        {"poketch-landmarks","Jubilife Poketch landmarks",match_poketch_landmarks},
        {"oreburgh-mart","Oreburgh Mart emblem",match_oreburgh_mart},
        {"jubilife-center","Jubilife Pokemon Center entrance",match_jubilife_center_entrance},
        {"eterna-center","Eterna Pokemon Center doorway",match_eterna_center_entrance},
        {"hearthome-center","Hearthome Pokemon Center doorway",match_hearthome_center_entrance},
        {"hearthome-center-interior","Hearthome Pokemon Center interior",match_hearthome_center_interior},
        {"hearthome-fantina","Fantina outside the Hearthome Contest Hall",match_hearthome_contest_fantina},
        {"route208-stair-top","Route 208 stair-top rocks",match_route208_stair_top},
        {"route208-hannah-departure","Route 208 field after Hannah",match_route208_hannah_departure},
        {"eterna-gym","Eterna Gym doorway",match_eterna_gym_entrance},
        {"galactic-hq-jupiter","Galactic HQ Jupiter interaction",match_galactic_hq_jupiter},
        {"eterna-gym-statues","Eterna Gym interior statues",match_eterna_gym_statues},
        {"eterna-underground-house","Eterna Underground Man house doorway",match_eterna_underground_house},
        {"eterna-underground-man","Eterna Underground Man interaction",match_eterna_underground_man},
        {"route201-potion-exit","Route 201 after Potion pickup",match_route201_potion_exit},
        {"sandgem-junction","Sandgem Center/Mart junction",match_sandgem_junction},
        {"route202-grass-exit","Route 202 after catching: grass exit",match_route202_grass_exit},
    };
    return matchers;
}
}}}
