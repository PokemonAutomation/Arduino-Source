/*  Python Bindings: `_pa_core` Module
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  pybind11 module that lets Python act as a Switch controller (serial port +
 *  PABotBase2 + controller scheduling from CoreLib). It is built only from
 *  this codebase: no Qt, OpenCV or Tesseract. Video capture and OCR live in the
 *  pure-Python `pokemon_automation` package next to this file, using opencv-python
 *  and pytesseract.
 *
 *  The controller class is `NintendoSwitch::PybindSwitchProController`
 *  (Source/Integrations/PybindSwitchController.h). Methods are bound under the names
 *  the Python package uses (e.g. `push_button()` as `press_buttons()`).
 *
 *  This is a thin 1:1 layer. Button-name parsing, input sequences and the MCP server
 *  are in the Python package, which imports this module as
 *  `pokemon_automation._pa_core`.
 *
 *  Conventions:
 *  - All durations are milliseconds.
 *  - Every call that can block (waiting for the device or a full command queue)
 *    releases the GIL, so other Python threads (e.g. an emergency stop) keep running.
 *  - C++ `PokemonAutomation::Exception`s become Python `RuntimeError`s.
 */

#include <deque>
#include <fstream>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Logging/MultiOutputLogger.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "Integrations/PybindSwitchController.h"

namespace py = pybind11;

namespace PokemonAutomation{

//  Defined by every executable that links CoreLib. The Python module has no Qt UI.
bool USE_QT_UI = false;

namespace PythonBindings{
namespace{



// Receives every line written to the global logger and forwards it to stderr and/or a
// log file, while keeping the most recent lines in memory for `recent_logs()`.
//
// Nothing is ever written to stdout: we will use this Pybind module to run an AI agent
// MCP server. When the MCP server runs over stdio, stdout carries the JSON-RPC protocol
// and any stray line would corrupt it.
class PythonLogSink : public Logger{
public:
    static PythonLogSink& instance(){
        static PythonLogSink sink;
        return sink;
    }

    void set_stderr(bool enabled){
        std::lock_guard<Mutex> lg(m_lock);
        m_stderr = enabled;
    }
    // Append log lines to `path`. An empty path stops file logging.
    // Throws FileException if the file can't be opened.
    void set_file(const std::string& path){
        std::lock_guard<Mutex> lg(m_lock);
        m_file.close();
        if (!path.empty()){
            m_file.open(path, std::ios::app);
            if (!m_file){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to open log file.", path);
            }
        }
    }
    std::vector<std::string> recent(size_t count) const{
        std::lock_guard<Mutex> lg(m_lock);
        count = std::min(count, m_recent.size());
        return std::vector<std::string>(m_recent.end() - count, m_recent.end());
    }

    //  Lines from `TaggedLogger`s already start with a timestamp.
    virtual void log(const std::string& msg, Color color = Color()) override{
        std::string line = msg;
        std::lock_guard<Mutex> lg(m_lock);
        if (m_stderr){
            std::cerr << line << std::endl;
        }
        if (m_file.is_open()){
            m_file << line << std::endl;
        }
        m_recent.emplace_back(std::move(line));
        if (m_recent.size() > 1000){
            m_recent.pop_front();
        }
    }

private:
    PythonLogSink(){
        global_multi_logger().add_listener(*this);
    }

    mutable Mutex m_lock;
    bool m_stderr = false;
    std::ofstream m_file;
    std::deque<std::string> m_recent;
};



}
}
}


using namespace PokemonAutomation;
using namespace PokemonAutomation::PythonBindings;
using NintendoSwitch::PybindSwitchProController;


PYBIND11_MODULE(_pa_core, m){
    m.doc() = "Pokemon Automation: act as a Nintendo Switch controller through a PABotBase2 serial device.";

    py::register_exception_translator([](std::exception_ptr p){
        try{
            if (p){
                std::rethrow_exception(p);
            }
        }catch (const PokemonAutomation::Exception& e){
            PyErr_SetString(PyExc_RuntimeError, e.to_str().c_str());
        }
    });

    //  Make sure the log sink is attached before anything logs.
    PythonLogSink::instance();


    //  Logging

    m.def(
        "set_log_stderr",
        [](bool enabled){ PythonLogSink::instance().set_stderr(enabled); },
        py::arg("enabled"),
        "Echo internal log lines to stderr."
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
        "Write a line to the internal log, so Python events appear alongside C++ ones."
    );
    m.def(
        "recent_logs",
        [](size_t count){ return PythonLogSink::instance().recent(count); },
        py::arg("count") = 50,
        "Return up to `count` of the most recent internal log lines."
    );


    //  Button constants, so the Python side never hard-codes bit positions.
    //  Keys are the C++ enum names without the "BUTTON_" prefix, e.g. "A", "ZL",
    //  "PLUS", "LCLICK", "HOME", "UP".

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


    //  Controller

    py::class_<PybindSwitchProController>(m, "Controller",
        "A Switch controller on a PABotBase2 device. Commands are queued and return "
        "immediately; call wait_for_all() to block until they have executed.")
        .def(py::init<const std::string&>(), py::arg("port_name"))
        .def("wait_for_ready", &PybindSwitchProController::wait_for_ready,
             py::arg("timeout_ms"), py::call_guard<py::gil_scoped_release>())
        .def("is_ready", &PybindSwitchProController::is_ready)
        .def("status_text", &PybindSwitchProController::current_status)
        .def("controller_name", &PybindSwitchProController::controller_name)
        .def("wait_for_all", &PybindSwitchProController::wait_for_all_requests,
             py::call_guard<py::gil_scoped_release>())
        .def("cancel_all", &PybindSwitchProController::cancel_all,
             py::call_guard<py::gil_scoped_release>())
        .def("release_all", &PybindSwitchProController::release_all,
             py::arg("timeout_ms"), py::call_guard<py::gil_scoped_release>())
        .def("wait", &PybindSwitchProController::wait,
             py::arg("duration_ms"), py::call_guard<py::gil_scoped_release>())
        .def("press_buttons", &PybindSwitchProController::push_button,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("buttons"),
             py::call_guard<py::gil_scoped_release>())
        .def("press_dpad", &PybindSwitchProController::push_dpad,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("position"),
             py::call_guard<py::gil_scoped_release>())
        .def("move_left_joystick", &PybindSwitchProController::push_left_joystick,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("x"), py::arg("y"),
             py::call_guard<py::gil_scoped_release>())
        .def("move_right_joystick", &PybindSwitchProController::push_right_joystick,
             py::arg("delay_ms"), py::arg("hold_ms"), py::arg("release_ms"), py::arg("x"), py::arg("y"),
             py::call_guard<py::gil_scoped_release>())
        .def("set_state", &PybindSwitchProController::controller_state,
             py::arg("duration_ms"), py::arg("buttons"), py::arg("dpad"),
             py::arg("left_x"), py::arg("left_y"), py::arg("right_x"), py::arg("right_y"),
             py::call_guard<py::gil_scoped_release>());
}
