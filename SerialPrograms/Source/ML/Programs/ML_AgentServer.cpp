/*  ML AI Agent Server Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <random>
#include <thread>
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ScopeExit.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/OCR/OCR_RawTesseractOCR.h"
#include "Controllers/ControllerRelease.h"
#include "GameConsole/Framework/ConsoleSystemSession.h"
#include "Integrations/AgentServer/AgentServer_HttpServer.h"
#include "Integrations/AgentServer/AgentServer_InputSteps.h"
#include "Integrations/AgentServer/AgentServer_McpServer.h"
#include "Integrations/AgentServer/AgentServer_ToolDefinitions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "ML_AgentServer.h"

namespace PokemonAutomation{
namespace ML{

using namespace NintendoSwitch;
using AgentServer::InputStep;
using AgentServer::McpContent;
using AgentServer::McpToolResult;
using nlohmann::json;



AgentServer_Descriptor::AgentServer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "ML:AgentServer",
        "ML", "AI Agent Server",
        "",
        "Let AI agents (Claude, Codex, ...) control the Switch over MCP while you watch "
        "and take over with the keyboard at any time.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}

struct AgentServer_Descriptor::Stats : public StatsTracker{
    Stats()
        : tool_calls(m_stats["Tool Calls"])
        , inputs(m_stats["Input Calls"])
        , screenshots(m_stats["Screenshots"])
        , takeovers(m_stats["User Takeovers"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Tool Calls");
        m_display_order.emplace_back("Input Calls");
        m_display_order.emplace_back("Screenshots");
        m_display_order.emplace_back("User Takeovers", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& tool_calls;
    std::atomic<uint64_t>& inputs;
    std::atomic<uint64_t>& screenshots;
    std::atomic<uint64_t>& takeovers;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> AgentServer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



namespace{

std::string random_token(){
    std::random_device rd;
    std::mt19937_64 rng(((uint64_t)rd() << 32) ^ rd());
    const char* digits = "0123456789abcdef";
    std::string ret;
    for (int c = 0; c < 32; c++){
        ret += digits[rng() & 15];
    }
    return ret;
}

int64_t now_ms(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

//  Encode an image as JPEG and return it base64-encoded, for an MCP image block.
std::string encode_jpeg_base64(const ImageViewRGB32& image, int quality){
    //  ImageRGB32 pixels are 0xAARRGGBB, i.e. QImage::Format_RGB32 (alpha ignored).
    QImage qimage(
        (const uchar*)image.data(),
        (int)image.width(), (int)image.height(),
        (qsizetype)image.bytes_per_row(),
        QImage::Format_RGB32
    );
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    qimage.save(&buffer, "JPG", quality);
    return bytes.toBase64().toStdString();
}

ImageFloatBox to_box(const json& value){
    if (!value.is_array() || value.size() != 4){
        return ImageFloatBox(0, 0, 1, 1);
    }
    return ImageFloatBox(value[0].get<double>(), value[1].get<double>(), value[2].get<double>(), value[3].get<double>());
}

}



//  The state of one run of the program: the MCP tool implementations, the queue
//  that moves input work onto the program thread, and who is in control.
//
//  Threads:
//  - Program thread: `run()` executes input jobs on the program's controller context,
//    so agent inputs go through the same scheduler as any automation program.
//  - HTTP worker threads: `call_tool()`. Observation tools (screenshot, read_text,
//    status, logs) run here directly; input tools are queued for the program thread.
//    `release_all` runs here directly so it works while an input job is running.
//  - UI thread: `on_controller_input()` (keyboard) and the program's buttons.
class AgentSession final : public AgentServer::McpToolHandler,
                           public GameConsole::ConsoleSystemSession::Listener{
public:
    enum class Control{ AGENT, USER };

    AgentSession(AgentServerProgram& options, SingleSwitchProgramEnvironment& env, ProControllerContext& context)
        : m_options(options)
        , m_env(env)
        , m_context(context)
        , m_stats(env.current_stats<AgentServer_Descriptor::Stats>())
        , m_settle_ms(options.SETTLE_MS)
        , m_max_hold_ms(options.MAX_HOLD_MS)
        , m_max_sequence_ms(options.MAX_SEQUENCE_MS)
        , m_screenshot_width(options.SCREENSHOT_WIDTH)
        , m_jpeg_quality(options.JPEG_QUALITY)
    {
        m_env.console.system_session().add_listener(*this);
    }
    ~AgentSession(){
        m_env.console.system_session().remove_listener(*this);
    }

    //  Program thread: execute queued input jobs until the program is stopped.
    //  Throws (like any program) when the user presses Stop.
    void run();

    //  Stop accepting tool calls and fail the queued ones. Any thread.
    void shutdown();

    //  Give control back to the agent (button, or idle timeout). Any thread.
    void return_control(const std::string& reason);

    //  HTTP worker threads.
    virtual McpToolResult call_tool(const std::string& name, const json& arguments) override;

    //  UI thread: the user's keyboard input was sent to this console's controller.
    virtual void on_controller_input(const ControllerInputState& state) override;


private:
    struct Job{
        std::function<McpToolResult()> work;
        std::promise<McpToolResult> result;
    };

    McpToolResult call_tool_unchecked(const std::string& name, const json& arguments);
    McpToolResult run_on_program_thread(std::function<McpToolResult()> work);

    McpToolResult tool_status();
    McpToolResult tool_release_all();
    McpToolResult tool_get_logs(const json& arguments);
    McpToolResult tool_screenshot(const json& box, uint32_t max_width, uint64_t wait_ms);
    McpToolResult tool_read_text(const json& arguments);
    McpToolResult tool_inputs(std::vector<InputStep> steps, const json& arguments, std::string description);

    //  Program thread.
    McpToolResult execute_inputs(const std::vector<InputStep>& steps, bool observe, uint64_t settle_ms, const std::string& description);
    void issue_step(const InputStep& step);

    //  A frame captured at or after `not_before`, cropped to `box`, downscaled to at
    //  most `max_width`, as [text, image] content. Throws InternalProgramError if the
    //  video is unavailable.
    std::vector<McpContent> capture(const json& box, uint32_t max_width, WallClock not_before);

    //  Sleep `ms`, waking early if the session shuts down. Returns false if it did.
    bool sleep_unless_stopped(uint64_t ms);

    //  Log to the program log, the video overlay and the `get_logs` history.
    void add_event(const std::string& text, Color color);

    std::string no_control_message() const{
        return "The user has taken control of the Switch, so your inputs were not sent. "
               "Stop sending inputs; call switch_status later to see when control returns to you.";
    }

private:
    AgentServerProgram& m_options;
    SingleSwitchProgramEnvironment& m_env;
    ProControllerContext& m_context;
    AgentServer_Descriptor::Stats& m_stats;

    //  Option values, read once: options are locked while the program runs.
    const uint32_t m_settle_ms;
    const uint32_t m_max_hold_ms;
    const uint32_t m_max_sequence_ms;
    const uint32_t m_screenshot_width;
    const uint32_t m_jpeg_quality;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::deque<std::shared_ptr<Job>> m_jobs;
    bool m_stopping = false;

    std::atomic<Control> m_control{Control::AGENT};
    //  Incremented by release_all and user takeovers; an input job stops issuing
    //  inputs as soon as it sees a change.
    std::atomic<uint64_t> m_abort_generation{0};
    std::atomic<int64_t> m_last_keyboard_ms{0};
    std::atomic<bool> m_keyboard_neutral{true};
    std::atomic<bool> m_stats_dirty{false};

    std::mutex m_events_lock;
    std::deque<std::string> m_events;
};



void AgentSession::add_event(const std::string& text, Color color){
    m_env.log("[Agent] " + text, color);
    m_env.console.overlay().add_log(text, color);
    std::lock_guard<std::mutex> lg(m_events_lock);
    m_events.emplace_back(current_time_to_str() + " - " + text);
    if (m_events.size() > 500){
        m_events.pop_front();
    }
}


void AgentSession::run(){
    while (true){
        m_context.throw_if_cancelled();

        if (m_stats_dirty.exchange(false)){
            m_env.update_stats();
        }

        //  Optional: return control after the keyboard has been idle for a while.
        uint32_t auto_return = m_options.AUTO_RETURN_SECONDS;
        if (auto_return > 0 &&
            m_control.load() == Control::USER &&
            m_keyboard_neutral.load() &&
            now_ms() - m_last_keyboard_ms.load() > (int64_t)auto_return * 1000
        ){
            return_control("no keyboard input for " + std::to_string(auto_return) + " seconds");
        }

        std::shared_ptr<Job> job;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait_for(lg, std::chrono::milliseconds(100), [&]{ return !m_jobs.empty() || m_stopping; });
            if (m_stopping){
                return;
            }
            if (m_jobs.empty()){
                continue;
            }
            job = std::move(m_jobs.front());
            m_jobs.pop_front();
        }

        try{
            job->result.set_value(job->work());
        }catch (ProgramCancelledException&){
            job->result.set_value(McpToolResult::error("The AI Agent Server was stopped."));
            throw;
        }catch (OperationCancelledException&){
            job->result.set_value(McpToolResult::error("The AI Agent Server was stopped."));
            throw;
        }catch (Exception& e){
            m_stats.errors++;
            job->result.set_value(McpToolResult::error(e.message()));
        }catch (std::exception& e){
            m_stats.errors++;
            job->result.set_value(McpToolResult::error(e.what()));
        }
        m_env.update_stats();
    }
}

void AgentSession::shutdown(){
    std::deque<std::shared_ptr<Job>> jobs;
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        jobs.swap(m_jobs);
    }
    m_cv.notify_all();
    for (auto& job : jobs){
        job->result.set_value(McpToolResult::error("The AI Agent Server was stopped."));
    }
}

void AgentSession::return_control(const std::string& reason){
    if (m_control.exchange(Control::AGENT) == Control::USER){
        add_event("Control returned to the agent (" + reason + ").", COLOR_BLUE);
    }
}

void AgentSession::on_controller_input(const ControllerInputState& state){
    m_last_keyboard_ms.store(now_ms());
    bool neutral = state.is_neutral();
    m_keyboard_neutral.store(neutral);
    if (neutral){
        return;
    }
    if (m_control.exchange(Control::USER) == Control::AGENT){
        //  The keyboard already overrides the controller's queued commands; this
        //  stops a running input job from issuing more and refuses new ones.
        m_abort_generation++;
        m_stats.takeovers++;
        m_stats_dirty.store(true);
        add_event("You took control. Agent inputs are paused until you click \"Return control to agent\".", COLOR_ORANGE);
    }
}


McpToolResult AgentSession::run_on_program_thread(std::function<McpToolResult()> work){
    auto job = std::make_shared<Job>();
    job->work = std::move(work);
    std::future<McpToolResult> future = job->result.get_future();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_stopping){
            return McpToolResult::error("The AI Agent Server is stopping.");
        }
        m_jobs.emplace_back(job);
    }
    m_cv.notify_all();
    return future.get();
}


McpToolResult AgentSession::call_tool(const std::string& name, const json& arguments){
    m_stats.tool_calls++;
    m_stats_dirty.store(true);
    try{
        McpToolResult result = call_tool_unchecked(name, arguments);
        if (result.is_error){
            m_stats.errors++;
        }
        return result;
    }catch (AgentServer::InputError& e){
        m_stats.errors++;
        return McpToolResult::error(e.what());
    }catch (Exception& e){
        m_stats.errors++;
        return McpToolResult::error(e.message());
    }
}

McpToolResult AgentSession::call_tool_unchecked(const std::string& name, const json& arguments){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_stopping){
            return McpToolResult::error("The AI Agent Server is stopping.");
        }
    }

    if (name == "switch_status"){
        return tool_status();
    }
    if (name == "release_all"){
        return tool_release_all();
    }
    if (name == "get_logs"){
        return tool_get_logs(arguments);
    }
    if (name == "screenshot"){
        return tool_screenshot(arguments.value("box", json()), arguments.value("max_width", m_screenshot_width), 0);
    }
    if (name == "wait_and_observe"){
        return tool_screenshot(arguments.value("box", json()), m_screenshot_width, arguments["duration_ms"].get<uint64_t>());
    }
    if (name == "read_text"){
        return tool_read_text(arguments);
    }

    //  Input tools
    if (name == "press_buttons"){
        json step = {
            {"buttons", arguments["buttons"]},
            {"hold_ms", arguments["hold_ms"]},
            {"release_ms", arguments["release_ms"]},
            {"repeat", arguments["repeat"]},
        };
        uint64_t repeat = arguments["repeat"].get<uint64_t>();
        std::string description = "press " + arguments["buttons"].get<std::string>() +
            (repeat > 1 ? " x" + std::to_string(repeat) : "");
        return tool_inputs({AgentServer::parse_step(step)}, arguments, description);
    }
    if (name == "move_stick"){
        std::string stick = arguments["stick"].get<std::string>();
        json step = {
            {stick + "_stick", arguments["direction"]},
            {"hold_ms", arguments["duration_ms"]},
            {"release_ms", 0},
        };
        if (arguments.contains("buttons")){
            step["buttons"] = arguments["buttons"];
        }
        AgentServer::parse_stick(arguments["direction"]);   //  clear error for a bad direction
        std::string description = stick + " stick " + arguments["direction"].dump() + " for " +
            std::to_string(arguments["duration_ms"].get<uint64_t>()) + " ms" +
            (arguments.contains("buttons") ? " holding " + arguments["buttons"].get<std::string>() : "");
        return tool_inputs({AgentServer::parse_step(step)}, arguments, description);
    }
    if (name == "run_inputs"){
        std::vector<InputStep> steps;
        for (const json& step : arguments["steps"]){
            steps.emplace_back(AgentServer::parse_step(step));
        }
        std::string description = std::to_string(steps.size()) + " step(s)";
        return tool_inputs(std::move(steps), arguments, description);
    }

    return McpToolResult::error("Tool " + name + " is not available in SerialPrograms.");
}


McpToolResult AgentSession::tool_status(){
    AbstractController& controller = m_env.console.controller();
    VideoSnapshot snapshot = m_env.console.video().snapshot();

    json status;
    status["control"] = m_control.load() == Control::AGENT ? "agent" : "user";
    status["controller"] = {
        {"ready", controller.is_ready()},
        {"name", controller.name()},
    };
    if (snapshot){
        status["video"] = {
            {"available", true},
            {"resolution", {snapshot.frame->width(), snapshot.frame->height()}},
        };
    }else{
        status["video"] = {{"available", false}};
    }
    status["limits"] = {
        {"max_hold_ms", m_max_hold_ms},
        {"max_sequence_ms", m_max_sequence_ms},
    };
    status["default_settle_ms"] = m_settle_ms;
    status["host"] = "SerialPrograms " + PROGRAM_VERSION;
    return McpToolResult::text(status.dump(2));
}


McpToolResult AgentSession::tool_release_all(){
    m_abort_generation++;
    bool confirmed = release_all_and_confirm(m_env.console.controller(), Milliseconds(1000));
    add_event(std::string("release_all (confirmed: ") + (confirmed ? "yes" : "no") + ")", COLOR_ORANGE);
    if (confirmed){
        return McpToolResult::text("All inputs released; the device confirmed the neutral state.");
    }
    return McpToolResult::text(
        "Release requested, but the device did not confirm within 1000 ms. "
        "Check switch_status; the controller may be disconnected or the Switch asleep."
    );
}


McpToolResult AgentSession::tool_get_logs(const json& arguments){
    size_t count = arguments["count"].get<size_t>();
    std::string text;
    {
        std::lock_guard<std::mutex> lg(m_events_lock);
        size_t start = m_events.size() > count ? m_events.size() - count : 0;
        for (size_t c = start; c < m_events.size(); c++){
            text += m_events[c] + "\n";
        }
    }
    return McpToolResult::text(text.empty() ? "(no events yet)" : text);
}


bool AgentSession::sleep_unless_stopped(uint64_t ms){
    std::unique_lock<std::mutex> lg(m_lock);
    return !m_cv.wait_for(lg, std::chrono::milliseconds(ms), [&]{ return m_stopping; });
}


std::vector<McpContent> AgentSession::capture(const json& box, uint32_t max_width, WallClock not_before){
    VideoSnapshot snapshot = m_env.console.video().snapshot();
    WallClock deadline = current_time() + std::chrono::seconds(1);
    while (snapshot && snapshot.timestamp < not_before && current_time() < deadline){
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        snapshot = m_env.console.video().snapshot();
    }
    if (!snapshot){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "No video. Select the capture card in the AI Agent Server program's video panel."
        );
    }

    ImageViewRGB32 view = *snapshot.frame;
    if (!box.is_null()){
        view = extract_box_reference(view, to_box(box));
    }
    std::string base64;
    size_t width = view.width();
    size_t height = view.height();
    if (width > max_width && max_width > 0){
        height = std::max<size_t>(1, height * max_width / width);
        width = max_width;
        ImageRGB32 scaled = view.scale_to(width, height);
        base64 = encode_jpeg_base64(scaled, (int)m_jpeg_quality);
    }else{
        base64 = encode_jpeg_base64(view, (int)m_jpeg_quality);
    }
    m_stats.screenshots++;
    m_stats_dirty.store(true);

    std::string text = "Screenshot " + std::to_string(width) + "x" + std::to_string(height) +
        " (full frame " + std::to_string(snapshot.frame->width()) + "x" + std::to_string(snapshot.frame->height()) + ")";
    return {
        McpContent::make_text(std::move(text)),
        McpContent::make_image(std::move(base64), "image/jpeg"),
    };
}


McpToolResult AgentSession::tool_screenshot(const json& box, uint32_t max_width, uint64_t wait_ms){
    if (wait_ms > 0 && !sleep_unless_stopped(wait_ms)){
        return McpToolResult::error("The AI Agent Server was stopped.");
    }
    McpToolResult result;
    result.content = capture(box, max_width, current_time());
    return result;
}


McpToolResult AgentSession::tool_read_text(const json& arguments){
    std::string code = arguments["language"].get<std::string>();
    Language language;
    try{
        language = language_code_to_enum(code);
    }catch (Exception&){
        return McpToolResult::error("Unknown OCR language \"" + code + "\". Use a Tesseract code such as \"eng\" or \"jpn\".");
    }
    if (!OCR::tesseract_language_available(language)){
        return McpToolResult::error(
            "OCR data for language \"" + code + "\" is not installed. Ask the user to download the "
            "\"Tesseract\" resource in SerialPrograms' Settings (resource downloads)."
        );
    }

    std::string mode = arguments["mode"].get<std::string>();
    OCR::PageSegMode psm = OCR::PageSegMode::SINGLE_BLOCK;
    if (mode == "line"){
        psm = OCR::PageSegMode::SINGLE_LINE;
    }else if (mode == "word"){
        psm = OCR::PageSegMode::SINGLE_WORD;
    }else if (mode == "sparse"){
        psm = (OCR::PageSegMode)11;     //  Tesseract PSM_SPARSE_TEXT
    }

    VideoSnapshot snapshot = m_env.console.video().snapshot();
    if (!snapshot){
        return McpToolResult::error("No video. Select the capture card in the AI Agent Server program's video panel.");
    }
    ImageViewRGB32 view = *snapshot.frame;
    if (arguments.contains("box")){
        view = extract_box_reference(view, to_box(arguments["box"]));
    }
    std::string text = OCR::tesseract_ocr_read(language, view, psm);
    size_t start = text.find_first_not_of(" \t\r\n");
    size_t end = text.find_last_not_of(" \t\r\n");
    text = start == std::string::npos ? "" : text.substr(start, end - start + 1);
    return McpToolResult::text(text.empty() ? "(no text found)" : text);
}


McpToolResult AgentSession::tool_inputs(std::vector<InputStep> steps, const json& arguments, std::string description){
    if (m_control.load() == Control::USER){
        return McpToolResult::error(no_control_message());
    }

    uint64_t total = 0;
    for (const InputStep& step : steps){
        if (!step.wait_only && step.hold_ms > m_max_hold_ms){
            return McpToolResult::error(
                "hold_ms " + std::to_string(step.hold_ms) + " exceeds the limit of " +
                std::to_string(m_max_hold_ms) + " ms."
            );
        }
        total += step.duration_ms();
    }
    if (total > m_max_sequence_ms){
        return McpToolResult::error(
            "The sequence lasts " + std::to_string(total) + " ms, over the limit of " +
            std::to_string(m_max_sequence_ms) + " ms. Split it into several calls."
        );
    }

    bool observe = arguments["observe"].get<bool>();
    uint64_t settle_ms = arguments.contains("settle_ms") ? arguments["settle_ms"].get<uint64_t>() : m_settle_ms;
    return run_on_program_thread([this, steps = std::move(steps), observe, settle_ms, description, total]{
        McpToolResult result = execute_inputs(steps, observe, settle_ms, description);
        if (!result.is_error && !result.content.empty()){
            result.content[0].text = "Done: " + description + " (" + std::to_string(total) + " ms of input)." +
                (result.content.size() > 1 ? " " + result.content[0].text : "");
        }
        return result;
    });
}


void AgentSession::issue_step(const InputStep& step){
    if (step.wait_only){
        if (step.wait_ms > 0){
            pbf_wait(m_context, Milliseconds(step.wait_ms));
        }
        return;
    }
    const bool buttons = step.pressed.has_buttons();
    const bool dpad = step.pressed.has_dpad();
    const bool left = step.left_stick.has_value();
    const bool right = step.right_stick.has_value();
    Milliseconds hold(step.hold_ms);
    Milliseconds release(step.release_ms);
    Milliseconds cycle(step.hold_ms + step.release_ms);

    for (uint64_t c = 0; c < step.repeat; c++){
        //  Single-kind inputs use the dedicated commands, which let the scheduler
        //  apply per-button cooldowns exactly like `pbf_*()` automation.
        if (buttons && !dpad && !left && !right){
            m_context->issue_buttons(&m_context, cycle, hold, release, step.pressed.buttons);
        }else if (dpad && !buttons && !left && !right){
            m_context->issue_dpad(&m_context, cycle, hold, release, step.pressed.dpad);
        }else if (left && !buttons && !dpad && !right){
            m_context->issue_left_joystick(&m_context, cycle, hold, release, *step.left_stick);
        }else if (right && !buttons && !dpad && !left){
            m_context->issue_right_joystick(&m_context, cycle, hold, release, *step.right_stick);
        }else{
            m_context->issue_full_controller_state(
                &m_context, true, hold,
                step.pressed.buttons, step.pressed.dpad,
                step.left_stick.value_or(JoystickPosition{}),
                step.right_stick.value_or(JoystickPosition{})
            );
            if (step.release_ms > 0){
                pbf_wait(m_context, release);
            }
        }
    }
    if (step.wait_ms > 0){
        pbf_wait(m_context, Milliseconds(step.wait_ms));
    }
}


McpToolResult AgentSession::execute_inputs(
    const std::vector<InputStep>& steps, bool observe, uint64_t settle_ms, const std::string& description
){
    if (m_control.load() == Control::USER){
        return McpToolResult::error(no_control_message());
    }
    const uint64_t generation = m_abort_generation.load();
    add_event("Agent: " + description, COLOR_DARKGREEN);
    m_stats.inputs++;
    m_stats_dirty.store(true);

    WallClock issue_start = current_time();
    uint64_t total_ms = 0;
    for (const InputStep& step : steps){
        if (m_abort_generation.load() != generation){
            break;
        }
        issue_step(step);
        total_ms += step.duration_ms();
    }

    //  Wait for the inputs to play out. Don't just call wait_for_all_requests():
    //  it holds the controller's issue lock for the whole wait, and the user's
    //  keyboard input needs that lock, so the user couldn't take over (and the UI
    //  would freeze) until a long agent input finished. Sleep in short slices
    //  instead, stopping early on a takeover or release_all, and sync with the
    //  device only at the end, when little or nothing is left to wait for.
    WallClock expected_end = issue_start + Milliseconds(total_ms);
    while (m_abort_generation.load() == generation && current_time() < expected_end){
        m_context.wait_for(std::min<Milliseconds>(
            Milliseconds(20),
            std::chrono::duration_cast<Milliseconds>(expected_end - current_time()) + Milliseconds(1)
        ));
    }
    if (m_abort_generation.load() == generation){
        m_context.wait_for_all_requests();
    }
    if (m_abort_generation.load() != generation){
        return McpToolResult::error(
            m_control.load() == Control::USER
                ? "Interrupted: " + no_control_message()
                : std::string("Interrupted: release_all was called while the inputs were running.")
        );
    }

    McpToolResult result = McpToolResult::text("");
    if (observe){
        WallClock after = current_time() + Milliseconds(settle_ms);
        m_context.wait_for(Milliseconds(settle_ms));
        std::vector<McpContent> screenshot = capture(json(), m_screenshot_width, after);
        result.content[0].text = screenshot[0].text + ", taken " + std::to_string(settle_ms) + " ms after the inputs finished.";
        result.content.emplace_back(std::move(screenshot[1]));
    }
    return result;
}




AgentServerProgram::~AgentServerProgram(){
    PORT.remove_listener(*this);
    REQUIRE_TOKEN.remove_listener(*this);
    ACCESS_TOKEN.remove_listener(*this);
    ALLOW_NETWORK.remove_listener(*this);
    NEW_TOKEN.remove_listener(static_cast<ButtonListener&>(*this));
    RETURN_CONTROL.remove_listener(static_cast<ButtonListener&>(*this));
}
AgentServerProgram::AgentServerProgram()
    : PORT(
        "<b>Port:</b><br>The local port the MCP server listens on.",
        LockMode::LOCK_WHILE_RUNNING,
        8765, 1024, 65535
    )
    , ALLOW_NETWORK(
        "<b>Allow connections from other computers:</b><br>"
        "Off (recommended): only programs on this computer can connect.<br>"
        "On: anything on your network that has the access token can control your Switch.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , REQUIRE_TOKEN(
        "<b>Require access token:</b><br>"
        "Agents must send the token below. Keeps other programs and web pages from "
        "controlling your Switch.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , ACCESS_TOKEN(
        false,
        "<b>Access token:</b><br>Generated automatically if empty.",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "Generated when the server starts"
    )
    , NEW_TOKEN("", "Generate new access token")
    , CONNECTION_INFO("")
    , SETTLE_MS(
        "<b>Default settle time (ms):</b><br>"
        "After an agent's inputs finish, wait this long before taking the screenshot "
        "that shows their effect. Agents can override it per call.",
        LockMode::LOCK_WHILE_RUNNING,
        500, 0, 10000
    )
    , MAX_HOLD_MS(
        "<b>Max hold per input (ms):</b><br>Longest time an agent may hold an input in one step.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 1, 600000
    )
    , MAX_SEQUENCE_MS(
        "<b>Max input per call (ms):</b><br>Longest total input an agent may send in one call.",
        LockMode::LOCK_WHILE_RUNNING,
        60000, 1, 3600000
    )
    , SCREENSHOT_WIDTH(
        "<b>Screenshot width:</b><br>Screenshots sent to agents are downscaled to this width.",
        LockMode::LOCK_WHILE_RUNNING,
        1280, 64, 3840
    )
    , JPEG_QUALITY(
        "<b>Screenshot JPEG quality:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        75, 10, 100
    )
    , AUTO_RETURN_SECONDS(
        "<b>Auto-return control (seconds):</b><br>"
        "When you take over with the keyboard, the agent's inputs are paused. Give control "
        "back automatically after this many seconds without keyboard input. 0 = only with "
        "the button below.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 3600
    )
    , RETURN_CONTROL(
        "<b>Agent control:</b><br>After you take over with the keyboard, click to let the agent continue.",
        "Return control to agent"
    )
{
    PA_ADD_OPTION(PORT);
    PA_ADD_OPTION(ALLOW_NETWORK);
    PA_ADD_OPTION(REQUIRE_TOKEN);
    PA_ADD_OPTION(ACCESS_TOKEN);
    PA_ADD_OPTION(NEW_TOKEN);
    PA_ADD_OPTION(CONNECTION_INFO);
    PA_ADD_OPTION(SETTLE_MS);
    PA_ADD_OPTION(MAX_HOLD_MS);
    PA_ADD_OPTION(MAX_SEQUENCE_MS);
    PA_ADD_OPTION(SCREENSHOT_WIDTH);
    PA_ADD_OPTION(JPEG_QUALITY);
    PA_ADD_OPTION(AUTO_RETURN_SECONDS);
    PA_ADD_OPTION(RETURN_CONTROL);

    update_connection_info();
    PORT.add_listener(*this);
    REQUIRE_TOKEN.add_listener(*this);
    ACCESS_TOKEN.add_listener(*this);
    ALLOW_NETWORK.add_listener(*this);
    NEW_TOKEN.add_listener(static_cast<ButtonListener&>(*this));
    RETURN_CONTROL.add_listener(static_cast<ButtonListener&>(*this));
}


void AgentServerProgram::update_connection_info(){
    std::string url = "http://127.0.0.1:" + std::to_string((uint16_t)PORT) + "/mcp";
    std::string token = ACCESS_TOKEN;
    bool require_token = REQUIRE_TOKEN;

    std::string text = "<b>Connect an agent</b> (while this program is running):<br>";
    text += "MCP server URL: <b>" + url + "</b> (Streamable HTTP)<br>";
    if (require_token){
        text += "Header: <b>Authorization: Bearer " + (token.empty() ? std::string("&lt;token&gt;") : token) + "</b><br>";
    }
    text += "Claude Code: <code>claude mcp add --transport http switch " + url;
    if (require_token){
        text += " --header \"Authorization: Bearer " + (token.empty() ? std::string("&lt;token&gt;") : token) + "\"";
    }
    text += "</code>";
    if (require_token && token.empty()){
        text += "<br>(The token is generated when you press Start.)";
    }
    CONNECTION_INFO.set_text(std::move(text));
}

void AgentServerProgram::on_config_value_changed(void* object){
    update_connection_info();
}

void AgentServerProgram::on_press(ButtonCell& button){
    if (&button == &RETURN_CONTROL){
        std::lock_guard<std::mutex> lg(m_session_lock);
        if (m_session != nullptr){
            m_session->return_control("you clicked the button");
        }
        return;
    }
    if (&button == &NEW_TOKEN){
        std::lock_guard<std::mutex> lg(m_session_lock);
        if (m_session == nullptr){     //  can't change it while agents are connected
            ACCESS_TOKEN.set(random_token());
        }
        return;
    }
}


void AgentServerProgram::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (REQUIRE_TOKEN && ((std::string)ACCESS_TOKEN).empty()){
        ACCESS_TOKEN.set(random_token());
    }

    static const AgentServer::AgentToolDefinitions definitions(
        AgentServer::agent_tools_json_text(), "app"
    );

    AgentServer::McpServerConfig config;
    config.server_version = PROGRAM_VERSION;
    config.access_token = REQUIRE_TOKEN ? (std::string)ACCESS_TOKEN : "";
    config.localhost_only = !ALLOW_NETWORK;

    AgentSession session(*this, env, context);
    AgentServer::McpServer mcp(env.logger(), definitions, session, config);
    AgentServer::HttpServer http(env.logger(), [&mcp](const AgentServer::HttpRequest& request){
        return mcp.handle(request);
    });

    {
        std::lock_guard<std::mutex> lg(m_session_lock);
        m_session = &session;
    }
    NEW_TOKEN.set_enabled(false);   //  agents are using the current token
    //  On any exit (Stop, error): refuse new tool calls, fail queued ones, then stop
    //  the server, which waits for in-flight requests to return.
    //  This runs during stack unwinding, so it must not throw.
    ScopeExit cleanup([&]{
        session.shutdown();
        http.stop();
        {
            std::lock_guard<std::mutex> lg(m_session_lock);
            m_session = nullptr;
        }
        NEW_TOKEN.set_enabled(true);
        try{
            release_all_and_confirm(env.console.controller(), Milliseconds(500));
        }catch (...){}
    });

    std::string error = http.start(PORT, ALLOW_NETWORK);
    if (!error.empty()){
        throw UserSetupError(env.logger(), "Unable to start the AI agent server: " + error);
    }
    env.console.overlay().add_log(
        "AI agent server on port " + std::to_string(http.port()) + ". Waiting for an agent...",
        COLOR_WHITE
    );

    session.run();
}




}
}
