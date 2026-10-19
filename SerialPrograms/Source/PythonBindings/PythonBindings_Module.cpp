/*  _pa_core: Pokemon Automation's Python module
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  pybind11 module that lets Python act as a Nintendo Switch controller through a
 *  PABotBase2 serial device. It wraps `NintendoSwitch::PybindSwitchProController`
 *  (Source/Integrations/), which is part of CoreLib, so it links CoreLib only: no
 *  Qt, OpenCV, ONNX Runtime or Tesseract.
 *
 *  The `pokemon_automation` Python package next to this file is built on top of
 *  this module (see Source/PythonBindings/README.md).
 *
 *  Conventions:
 *  - All durations are milliseconds.
 *  - Commands are queued on the device: the push_*() / controller_state() calls
 *    return as soon as the command is scheduled, not when it executes. Call
 *    wait_for_all_requests() to block until everything queued has executed.
 *  - Every call that can block releases the Python GIL, so other Python threads
 *    (e.g. an emergency stop) keep running.
 *  - C++ `PokemonAutomation::Exception`s become Python `RuntimeError`s.
 *  - Nothing is written to stdout. Internal log lines can be routed to stderr
 *    and/or a log file with set_log_stderr() / set_log_file().
 */

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Logging/MultiOutputLogger.h"
#include "Common/Cpp/Time.h"
#include "Integrations/PybindSwitchController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

namespace py = pybind11;

//  CoreLib references this in every executable that links it. The module has no Qt UI.
namespace PokemonAutomation{
    bool USE_QT_UI = false;
}


namespace{

//  Receives every line written to the global logger and forwards it to stderr
//  and/or a log file, while keeping the most recent lines in memory for
//  `recent_logs()`.
class PythonLogSink final : public PokemonAutomation::Logger{
public:
    static PythonLogSink& instance(){
        static PythonLogSink sink;
        return sink;
    }

    void set_stderr(bool enabled){
        PokemonAutomation::LockGuard<PokemonAutomation::Mutex> lg(m_lock);
        m_stderr = enabled;
    }

    // Append log lines to `path`. An empty path stops file logging.
    // Throws FileException if the file can't be opened.
    void set_file(const std::string& path){
        PokemonAutomation::LockGuard<PokemonAutomation::Mutex> lg(m_lock);
        m_file.close();
        if (!path.empty()){
            m_file.open(path, std::ios::app);
            if (!m_file){
                throw PokemonAutomation::FileException(
                    nullptr,
                    PA_CURRENT_FUNCTION,
                    "Unable to open log file.",
                    path
                );
            }
        }
    }

    // Up to `count` of the most recent log lines, oldest first.
    std::vector<std::string> recent(size_t count) const{
        PokemonAutomation::LockGuard<PokemonAutomation::Mutex> lg(m_lock);
        count = std::min(count, m_recent.size());
        return std::vector<std::string>(m_recent.end() - count, m_recent.end());
    }

    virtual void log(const std::string& msg, PokemonAutomation::Color color = PokemonAutomation::Color()) override{
        PokemonAutomation::LockGuard<PokemonAutomation::Mutex> lg(m_lock);
        if (m_stderr){
            std::cerr << msg << std::endl;
        }
        if (m_file.is_open()){
            m_file << msg << std::endl;
            m_file.flush();
        }
        m_recent.emplace_back(msg);
        if (m_recent.size() > kMaxRecentLines){
            m_recent.pop_front();
        }
    }

private:
    PythonLogSink(){
        PokemonAutomation::global_multi_logger().add_listener(*this);
    }

    static constexpr size_t kMaxRecentLines = 1000;

    mutable PokemonAutomation::Mutex m_lock;
    bool m_stderr = true;
    std::ofstream m_file;
    std::deque<std::string> m_recent;
};

}


using namespace PokemonAutomation;
using NintendoSwitch::PybindSwitchProController;


PYBIND11_MODULE(_pa_core, m){
    m.doc() = "Pokemon Automation: act as a Nintendo Switch controller through a PABotBase2 serial device.";

    //  C++ exceptions -> Python exceptions.
    py::register_exception_translator([](std::exception_ptr p){
        try{
            if (p){
                std::rethrow_exception(p);
            }
        }catch (const PokemonAutomation::Exception& e){
            PyErr_SetString(PyExc_RuntimeError, e.to_str().c_str());
        }
    });

    //  Attach the log sink before anything logs.
    PythonLogSink::instance();


    //  Logging.

    m.def(
        "set_log_stderr",
        [](bool enabled){ PythonLogSink::instance().set_stderr(enabled); },
        py::arg("enabled"),
        "Echo internal log lines to stderr (enabled by default)."
    );
    m.def(
        "set_log_file",
        [](const std::string& path){ PythonLogSink::instance().set_file(path); },
        py::arg("path"),
        "Append internal log lines to this file. Pass an empty string to stop."
    );
    m.def(
        "log",
        [](const std::string& message){
            global_logger_raw().log(current_time_to_str() + " - " + message);
        },
        py::arg("message"),
        "Write a line to the internal log, so Python events appear alongside the C++ ones."
    );
    m.def(
        "recent_logs",
        [](size_t count){ return PythonLogSink::instance().recent(count); },
        py::arg("count") = 50,
        "Return up to `count` of the most recent internal log lines, oldest first."
    );


    //  Input constants, so Python code never hard-codes bit positions or d-pad
    //  numbers. Keys are the C++ names without the "BUTTON_" / "DPAD_" prefix.

    py::dict buttons;
    for (size_t bit = 0; bit < NintendoSwitch::TOTAL_BUTTONS; bit++){
        NintendoSwitch::Button button = (NintendoSwitch::Button)((uint32_t)1 << bit);
        std::string name = NintendoSwitch::button_to_code_string(button);
        if (name.starts_with("BUTTON_")){
            name = name.substr(7);
        }
        buttons[py::str(name)] = (uint32_t)button;
    }
    m.attr("BUTTONS") = buttons;

    py::dict dpad_positions;
    for (size_t position = 0; position <= (size_t)NintendoSwitch::DpadPosition::DPAD_NONE; position++){
        std::string name = NintendoSwitch::dpad_to_code_string((NintendoSwitch::DpadPosition)position);
        if (name.starts_with("DPAD_")){
            name = name.substr(5);
        }
        dpad_positions[py::str(name)] = (uint32_t)position;
    }
    m.attr("DPAD_POSITIONS") = dpad_positions;


    //  Controller.

    py::class_<PybindSwitchProController>(m, "Controller",
        "A Switch controller on a PABotBase2 device. Commands are queued on the device "
        "and return as soon as they are scheduled; call wait_for_all_requests() to block "
        "until they have executed.")
        .def(py::init<const std::string&>(), py::arg("port_name"))
        .def("wait_for_ready", &PybindSwitchProController::wait_for_ready,
             py::arg("timeout_ms") = 10000,
             "Block until the device handshake finishes and a Switch controller is "
             "available, or `timeout_ms` passes. Returns True if ready.",
             py::call_guard<py::gil_scoped_release>())
        .def("is_ready", &PybindSwitchProController::is_ready,
             "True if the device is connected and a Switch controller is available.")
        .def("current_status", &PybindSwitchProController::current_status,
             "The connection status text: device name and firmware version, or the "
             "error message if the connection failed.")
        .def("controller_name", &PybindSwitchProController::controller_name,
             "Name of the active controller implementation, e.g. "
             "\"Nintendo Switch: Pro Controller\". Empty if not ready.")
        .def("wait_for_all_requests", &PybindSwitchProController::wait_for_all_requests,
             "Block until every command queued so far has been executed by the device.",
             py::call_guard<py::gil_scoped_release>())
        .def("cancel_all_and_wait", &PybindSwitchProController::cancel_all_and_wait,
             py::arg("timeout_ms") = 500,
             "Cancel every command that has not executed yet, clear the device's command "
             "queue, return the controller to the neutral state (no buttons pressed, "
             "sticks centered), and wait up to `timeout_ms` for the device to "
             "acknowledge. Returns True if acknowledged, False on timeout or if the "
             "controller is not ready. Thread-safe: may be called from another thread, "
             "e.g. as an emergency stop.",
             py::call_guard<py::gil_scoped_release>())
        .def("wait", &PybindSwitchProController::wait,
             py::arg("duration_ms"),
             "Queue a pause of `duration_ms` with nothing pressed.",
             py::call_guard<py::gil_scoped_release>())
        .def("push_button", &PybindSwitchProController::push_button,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("bitfield"),
             "Queue a press of every button in `bitfield` (see `BUTTONS`). `delay_ms` is "
             "how long to wait before the press may start, `hold_ms` how long the buttons "
             "stay down, `release_ms` how long they stay up before the same button can be "
             "pressed again. D-pad directions are not part of `bitfield`; use push_dpad().",
             py::call_guard<py::gil_scoped_release>())
        .def("push_dpad", &PybindSwitchProController::push_dpad,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("position"),
             "Queue a d-pad press in direction `position` (see `DPAD_POSITIONS`). "
             "`delay_ms` / `hold_ms` / `release_ms` have the same meaning as in push_button().",
             py::call_guard<py::gil_scoped_release>())
        .def("push_left_joystick", &PybindSwitchProController::push_left_joystick,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("x"), py::arg("y"),
             "Queue the left stick being held at (x, y), in [-1.0, 1.0] with +x = right "
             "and +y = up. `delay_ms` / `hold_ms` / `release_ms` have the same meaning as in push_button().",
             py::call_guard<py::gil_scoped_release>())
        .def("push_right_joystick", &PybindSwitchProController::push_right_joystick,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("x"), py::arg("y"),
             "Queue the right stick being held at (x, y), in [-1.0, 1.0] with +x = right "
             "and +y = up. `delay_ms` / `hold_ms` / `release_ms` have the same meaning as in push_button().",
             py::call_guard<py::gil_scoped_release>())
        .def("controller_state", &PybindSwitchProController::controller_state,
             py::arg("duration_ms"), py::arg("buttons"), py::arg("dpad"),
             py::arg("left_x"), py::arg("left_y"), py::arg("right_x"), py::arg("right_y"),
             "Queue the entire controller state (buttons, d-pad, both sticks) being held "
             "for `duration_ms`; everything not specified is released. This is the most "
             "direct way to hold arbitrary combinations, e.g. B while moving the left stick.",
             py::call_guard<py::gil_scoped_release>());
}
