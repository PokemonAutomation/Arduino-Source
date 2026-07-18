/*  Waterfill Template Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "WaterfillTemplateMaker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




WaterfillTemplateMaker_Descriptor::WaterfillTemplateMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:WaterfillTemplateMaker",
        "Nintendo Switch", "Waterfill Template Maker",
        "",
        "Helper to make waterfill templates.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}



WaterfillTemplateMaker::~WaterfillTemplateMaker(){
    BUTTON.remove_listener(*this);
}
WaterfillTemplateMaker::WaterfillTemplateMaker()
    : BUTTON("<b>Make Template:</b>", "Make Template")
    , MIN_AREA("<b>Min Area (in pixels):</b>", LockMode::UNLOCK_WHILE_RUNNING, 100)
    , FILTER_LOWER("<b>Filter (lower):</b>", LockMode::UNLOCK_WHILE_RUNNING, false, 0xff000000, 0xff000000)
    , FILTER_UPPER("<b>Filter (upper):</b>", LockMode::UNLOCK_WHILE_RUNNING, false, 0xffffffff, 0xffffffff)
    , BOX_DRAW(LockMode::UNLOCK_WHILE_RUNNING)
{
    PA_ADD_OPTION(BUTTON);
    PA_ADD_OPTION(MIN_AREA);
    PA_ADD_OPTION(FILTER_LOWER);
    PA_ADD_OPTION(FILTER_UPPER);
    PA_ADD_OPTION(BOX_DRAW);
    BUTTON.add_listener(*this);
}


void WaterfillTemplateMaker::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_stream = &env.console;
    }
    ScopeExit on_exit([this]{
        std::lock_guard<Mutex> lg(m_lock);
        m_stream = nullptr;
    });
    auto drawn_box = BOX_DRAW.make_session(env.console.overlay());
    scope.wait_until_cancel();
}


void WaterfillTemplateMaker::on_press(){

    using namespace Kernels;
    using namespace Kernels::Waterfill;

    global_logger_tagged().log("Button pressed. Attempting to make template.");
    std::lock_guard<Mutex> lg(m_lock);
    if (m_stream == nullptr){
        global_logger_tagged().log("Program isn't running.", COLOR_RED);
    }

    VideoSnapshot screenshot = m_stream->video().snapshot_latest_blocking();
    ImageViewRGB32 image;
    if (BOX_DRAW.CONTENT_BOX.enabled()){
        image = extract_box_reference(*screenshot.frame, BOX_DRAW.CONTENT_BOX);
    }
    image = extract_box_reference(*screenshot.frame, BOX_DRAW.INFERENCE_BOX);

    image.save("WaterfillTemplateMaker-0-SearchBox.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        FILTER_LOWER | 0xff000000,
        FILTER_UPPER | 0xff000000
    );

    PackedBinaryMatrix inverse = matrix.copy();
    inverse.invert();


    auto session = make_WaterfillSession();
    WaterfillObject largest_object;
    {
        session->set_source(matrix);
        auto finder = session->make_iterator(MIN_AREA);
        WaterfillObject object;
        while (finder->find_next(object, true)){
            if (object.area == 0 || largest_object.box_area() < object.box_area()){
                largest_object = std::move(object);
            }
        }
    }
    if (largest_object.area == 0){
        global_logger_tagged().log("Failed to find any waterfill objects.", COLOR_RED);
        return;
    }

//    cout << matrix.dump() << endl;

    ImageViewRGB32 cropped = extract_box_reference(image, largest_object);
    cropped.save("WaterfillTemplateMaker-1-FoundObject.png");

    //  Remove the background.
    ImageRGB32 stripped = image.copy();
    {
        session->set_source(inverse);
        auto finder = session->make_iterator(1);
        WaterfillObject object;
        if (!finder->find_next(object, true)){
            global_logger_tagged().log("Failed to waterfill inverse.", COLOR_RED);
            return;
        }
        filter_by_mask(object.packed_matrix(), stripped, Color(0), false);
    }
    stripped.save("WaterfillTemplateMaker-2-Stripped.png");


#if 0

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, MIN_AREA);
    if (objects.empty()){
        global_logger_tagged().log("Failed to find any waterfill objects.", COLOR_RED);
        return;
    }

    const WaterfillObject* largest_object = &objects[0];
    for (const WaterfillObject& object : objects){
        if (largest_object->area < object.area){
            largest_object = &object;
        }
    }

    extract_box_reference(image, *largest_object).save("image-cropped.png");

    cout << "Matrix: " << matrix.width() << " x " << matrix.height() << endl;
    cout << "Image:  " << image.width() << " x " << image.height() << endl;

    ImageRGB32 filtered = image.copy();
    {
        auto session = make_WaterfillSession(inverse);
        auto finder = session->make_iterator(MIN_AREA);
        WaterfillObject object;
        if (!finder->find_next(object, true)){
            global_logger_tagged().log("Failed to waterfill inverse.", COLOR_RED);
            return;
        }
        filter_by_mask(object.packed_matrix(), filtered, Color(0), false);
        filtered.save("image-filtered.png");
//        extract_box_reference(filtered, *largest_object).save("image-filtered.png");
    }

//    filter_by_mask(*largest_object->object., filtered, Color(0), true);
#endif

}

























}
}
