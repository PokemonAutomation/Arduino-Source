/*  Pokedex Tasks Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_PokedexTasksReader.h"

#include <array>
#include <fstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


class PokemonTasksReader{
public:
    PokemonTasksReader(VideoStream& stream)
        : m_stream(stream)
        , m_tasks_box{
            OverlayBoxScope(stream.overlay(), {0.400, 0.190, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.244, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.298, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.353, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.406, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.460, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.514, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.568, 0.040, 0.045}),
            OverlayBoxScope(stream.overlay(), {0.400, 0.622, 0.040, 0.045})
        }
    {}

    std::array<int, 9> read_tasks(const ImageViewRGB32& screen) const
    {
        std::array<int, 9> tasks{};
        for (size_t i = 0; i < m_tasks_box.size(); ++i)
        {
            ImageRGB32 image = to_blackwhite_rgb32_range(
                extract_box_reference(screen, m_tasks_box[i]),
                false,
                0xff808080, 0xffffffff
            );
            tasks[i] = OCR::read_number(m_stream.logger(), image);
        }
        return tasks;
    }

private:
    VideoStream& m_stream;
    std::array<OverlayBoxScope, 9> m_tasks_box;
};

PokedexTasksReader_Descriptor::PokedexTasksReader_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:PokedexTasksReader",
        STRING_POKEMON + " LA", STRING_POKEDEX + " Tasks Reader",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/PokedexTasksReader.md",
        "Read all the tasks in your " + STRING_POKEDEX + " and output a file with the tasks you did.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


PokedexTasksReader::PokedexTasksReader(){}


void PokedexTasksReader::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    std::ofstream output_file("output.txt");

    for (int i = 0; i < 242; ++i){
        PokemonTasksReader reader(env.console);
        std::array<int, 9> tasks = reader.read_tasks(env.console.video().snapshot());
        for (auto task : tasks){
            if (task != -1){
                output_file << task << "\n";
            }
        }
        pbf_press_dpad(context, DPAD_DOWN, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
