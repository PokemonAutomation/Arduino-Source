/*  Generate Pokemon Sprite Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <filesystem>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Resources/PokemonSwSh_NameDatabase.h"
#include "PokemonSwSh_GeneratePokedexSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


using namespace Pokemon;


GeneratePokedexSprites_Descriptor::GeneratePokedexSprites_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:GeneratePokedexSprites",
        STRING_POKEMON + " SwSh", "Generate " + STRING_POKEMON + " Sprite Data",
        "",
        "Generate " + STRING_POKEMON + " Sprite data by iterating the " + STRING_POKEDEX + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


GeneratePokedexSprites::GeneratePokedexSprites()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING
    )
    , HORIZONTAL_FRAMES(
        "<b>Frames per 360 Degree Horizontal Rotation:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        10
    )
    , ANIMATION_FRAMES(
        "<b>Animation Frames per Camera Angle:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HORIZONTAL_FRAMES);
    PA_ADD_OPTION(ANIMATION_FRAMES);
}


class GenerateDexModelSession{
    static constexpr uint16_t HORIZONTAL_360 = 160;
    static constexpr uint16_t VERTICAL_90 = 40;

public:
    GenerateDexModelSession(
        VideoStream& stream, ProControllerContext& context,
        size_t horizontal_frames = 10,
        size_t animation_frames = 2
    )
        : m_stream(stream), m_context(context)
        , m_path("PokedexSprites/")
        , m_horizontal_frames(horizontal_frames)
        , m_vertical_frames(horizontal_frames * 40 / HORIZONTAL_360 + 1)
        , m_animation_frames(animation_frames)
//        , m_frames_per_form(m_horizontal_frames * m_vertical_frames * (m_animation_frames + 1))
    {
//        cout << "frames/form = " << m_frames_per_form << endl;
    }


public:
    void save_image(const std::string& slug, bool is_shiny, size_t form_index, size_t image_index);
    void iterate_form(const std::string& slug, bool shiny, size_t form_index);
    void iterate_species();
    void iterate_dex();

private:
    VideoStream& m_stream;
    ProControllerContext& m_context;

    std::string m_path;

    size_t m_horizontal_frames;
    size_t m_vertical_frames;
    size_t m_animation_frames;
//    size_t m_frames_per_form;

    std::set<std::string> m_processed_slugs;
};




void GenerateDexModelSession::save_image(const std::string& slug, bool is_shiny, size_t form_index, size_t image_index){
    m_context.wait_for_all_requests();
    VideoSnapshot screen = m_stream.video().snapshot();
    ImageViewRGB32 cropped = extract_box_reference(screen, ImageFloatBox(0.20, 0.01, 0.60, 0.93));

    std::string filename = m_path + slug + "/" + slug;
    filename += is_shiny ? "_shiny" : "_nonshiny";
    filename += "_f" + std::to_string(form_index);
    filename += "_#" + std::to_string(image_index);
    filename += ".jpg";
    cropped.save(filename);
}
void GenerateDexModelSession::iterate_form(const std::string& slug, bool shiny, size_t form_index){
    size_t image_index = 0;

    uint16_t step = 160 / (uint16_t)m_horizontal_frames + 1;

    //  Stills
    for (size_t y = 0; y < m_vertical_frames; y++){
        for (size_t x = 0; x < m_horizontal_frames; x++){
            pbf_press_dpad(m_context, DPAD_RIGHT, step, 50);
            save_image(slug, shiny, form_index, image_index++);
        }
        pbf_press_dpad(m_context, DPAD_DOWN, step, 50);
    }
    pbf_press_dpad(m_context, DPAD_UP, 125, 0);

    //  Motion
    pbf_press_button(m_context, BUTTON_A, 20, 30);
    for (size_t y = 0; y < m_vertical_frames; y++){
        for (size_t x = 0; x < m_horizontal_frames; x++){
            pbf_press_dpad(m_context, DPAD_RIGHT, step, 50);
            for (size_t t = 0; t < m_animation_frames; t++){
                save_image(slug, shiny, form_index, image_index++);
                m_context.wait_for(std::chrono::milliseconds(500));
            }
        }
        pbf_press_dpad(m_context, DPAD_DOWN, step, 50);
    }
    pbf_press_dpad(m_context, DPAD_UP, 125, 0);
}
void GenerateDexModelSession::iterate_species(){
    ImageFloatBox SPRITE_BOX(0.45, 0.13, 0.06, 0.09);
    ImageFloatBox LABEL_BOX(0.85, 0.13, 0.05, 0.09);
    ImageFloatBox SHINY_BOX(0.90, 0.14, 0.03, 0.06);
    ImageRGB32 start_sprite;
    ImageRGB32 start_label;
    ImageRGB32 start_shiny;
    std::string slug;
    bool is_shiny;
    {
        VideoSnapshot screen = m_stream.video().snapshot();
        start_sprite = extract_box_reference(screen, SPRITE_BOX).copy();
        start_label = extract_box_reference(screen, LABEL_BOX).copy();
        start_shiny = extract_box_reference(screen, SHINY_BOX).copy();
        is_shiny = image_stats(start_shiny).stddev.sum() > 100;

        ImageFloatBox NAME_BOX(0.641, 0.134, 0.210, 0.080);
        PokemonNameReader reader(ALL_POKEMON_SLUGS());
        OCR::StringMatchResult result = reader.read_substring(
            m_stream.logger(), Language::English,
            extract_box_reference(screen, NAME_BOX),
            OCR::WHITE_TEXT_FILTERS()
        );
        if (result.results.empty()){
            m_stream.log("Unable to read " + STRING_POKEMON + "name.", COLOR_RED);
            return;
        }
        slug = result.results.begin()->second.token;
    }

#if 1
    if (m_processed_slugs.contains(slug)){
//        m_console.log("Skipping already processed " + STRING_POKEMON + ".", COLOR_BLUE);
//        return;
        throw ProgramFinishedException();
    }
    m_processed_slugs.insert(slug);
#endif

    std::filesystem::create_directories(m_path + slug);

    size_t non_shiny_index = 0;
    size_t shiny_index = 0;

    double rmsd;
    do{
//        cout << "shiny = " << is_shiny;

        size_t& index = is_shiny ? shiny_index : non_shiny_index;

        pbf_press_button(m_context, BUTTON_A, 20, 230);
        iterate_form(slug, is_shiny, index);
        pbf_press_button(m_context, BUTTON_B, 20, 230);
        pbf_press_dpad(m_context, DPAD_RIGHT, 20, 105);
        m_context.wait_for_all_requests();

        index++;

        VideoSnapshot screen = m_stream.video().snapshot();
        ImageViewRGB32 current_sprite = extract_box_reference(screen, SPRITE_BOX);
        ImageViewRGB32 current_label = extract_box_reference(screen, LABEL_BOX);
        ImageViewRGB32 current_shiny = extract_box_reference(screen, SHINY_BOX);
        is_shiny = image_stats(current_shiny).stddev.sum() > 100;
        double rmsd0 = ImageMatch::pixel_RMSD(start_sprite, current_sprite);
        double rmsd1 = ImageMatch::pixel_RMSD(start_label, current_label);
        double rmsd2 = ImageMatch::pixel_RMSD(start_shiny, current_shiny);
        rmsd = rmsd0 + rmsd1 + rmsd2;
//        cout << ", rmsd = " << rmsd << endl;
    }while (rmsd > 5);
}
void GenerateDexModelSession::iterate_dex(){
    while (true){
        iterate_species();
        pbf_press_dpad(m_context, DPAD_DOWN, 20, 105);
        m_context.wait_for_all_requests();
    }
}






void GeneratePokedexSprites::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    GenerateDexModelSession session(env.console, context);
    session.iterate_dex();
}



}
}
}
