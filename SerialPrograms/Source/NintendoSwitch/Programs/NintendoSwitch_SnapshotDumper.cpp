/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include <QKeyEvent>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch_SnapshotDumper.h"
// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


SnapshotDumper_Descriptor::SnapshotDumper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:SnapshotDumper",
        "Nintendo Switch", "Snapshot Dumper",
        "Programs/NintendoSwitch/SnapshotDumper.html",
        "Periodically take screenshots.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}

SnapshotDumper::~SnapshotDumper(){
    SNAPSHOT_MODE.remove_listener(*this);
}

SnapshotDumper::SnapshotDumper()
    : PERIOD_MILLISECONDS(
        "<b>Snapshot Period (milliseconds):</b><br>Take screenshot every this many milliseconds.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000
    )
    , SNAPSHOT_MODE(
        "<b>Snapshot trigger:",
        {
            {SnapshotMode::KEYPRESS,         "key-press",           "Key Press. Press 'Page Down', while the screen is focused."},
            {SnapshotMode::MOUSE_CLICK,            "mouse-click",              "Mouse click on the screen."},
            {SnapshotMode::PERIODIC,            "periodic",              "Periodic: every X milliseconds as defined below."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SnapshotMode::KEYPRESS
    ) 
    , FORMAT(
        "<b>Image Format:</b>",
        {
            {Format::PNG, "png", ".png"},
            {Format::JPG, "jpg", ".jpg"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        Format::JPG
    )
{
    PA_ADD_OPTION(SNAPSHOT_MODE);
    PA_ADD_OPTION(PERIOD_MILLISECONDS);
    PA_ADD_OPTION(FORMAT);
    SNAPSHOT_MODE.add_listener(*this);
}

void SnapshotDumper::on_config_value_changed(void* object){
    PERIOD_MILLISECONDS.set_visibility(SNAPSHOT_MODE == SnapshotMode::PERIODIC ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
}

class SnapshotClickTrigger : public VideoOverlay::MouseListener{
public:
    ~SnapshotClickTrigger(){
        detach();
    }
    SnapshotClickTrigger(VideoStream& stream, VideoOverlay& overlay, Format format)
        : m_stream(stream)
        , m_overlay(overlay)
        , m_format(format)
    {
        try{
            overlay.add_mouse_listener(*this);
        }catch (...){
            detach();
            throw;
        }
    }
    
    virtual void on_mouse_press(double x, double y) override{
        dump_snapshot(m_stream, "ScreenshotDumper", to_format_string(m_format));
    }
    

private:
    void detach(){
        m_overlay.remove_mouse_listener(*this);
    }

private:
    VideoStream& m_stream;
    VideoOverlay& m_overlay;
    Format m_format;

};


SnapshotKeyTrigger::~SnapshotKeyTrigger(){
    detach();
}
SnapshotKeyTrigger::SnapshotKeyTrigger(VideoStream& stream, VideoOverlay& overlay, Format format)
    : m_stream(stream)
    , m_overlay(overlay)
    , m_format(format)
{
    try{
        overlay.add_keyevent_listener(*this);
    }catch (...){
        detach();
        throw;
    }
}

void SnapshotKeyTrigger::detach(){
    m_overlay.remove_keyevent_listener(*this);
}

void SnapshotKeyTrigger::on_key_press(QKeyEvent* event){
    if (event->key() == Qt::Key::Key_PageDown){
        dump_snapshot(m_stream, "ScreenshotDumper", to_format_string(m_format));
    } 
}
void SnapshotKeyTrigger::on_key_release(QKeyEvent* event){
}


void SnapshotDumper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::string folder_path = USER_FILE_PATH() + "ScreenshotDumper/";
    QDir().mkpath(folder_path.c_str());

    if (SNAPSHOT_MODE == SnapshotMode::KEYPRESS){
        SnapshotKeyTrigger key_trigger(env.console, env.console.overlay(), FORMAT);
        context.wait_until_cancel();
    }else if (SNAPSHOT_MODE == SnapshotMode::MOUSE_CLICK){
        SnapshotClickTrigger click_trigger(env.console, env.console.overlay(), FORMAT);
        context.wait_until_cancel();
    }else if (SNAPSHOT_MODE == SnapshotMode::PERIODIC){
        while (true){
            VideoSnapshot last = env.console.video().snapshot();
            std::string filename = folder_path + now_to_filestring();
            last->save(filename + to_format_string(FORMAT));
            context.wait_until(last.timestamp + std::chrono::milliseconds(PERIOD_MILLISECONDS));
        }
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unexpected SNAPSHOT_MODE enum.");
    }
}

std::string to_format_string(Format format){
    switch (format){
    case Format::PNG:
        return ".png";
    case Format::JPG:
        return ".jpg";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "to_format_string: Unknown Format enum.");
        break;
    }
}

void dump_snapshot(VideoStream& stream, std::string folder_name, std::string format){
    std::string folder_path = USER_FILE_PATH() + folder_name + "/";
    QDir().mkpath(folder_path.c_str());
    VideoSnapshot last = stream.video().snapshot();
    std::string filename = folder_path + now_to_filestring() + format;
    last->save(filename);
}


}
}

