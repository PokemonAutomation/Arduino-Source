/*  Hardware Validation (x86)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <thread>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Environment.h"
#include "HardwareValidation.h"

namespace PokemonAutomation{


bool check_hardware(){
    if (!CPU_CAPABILITY_NATIVE.OK_08_Nehalem){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "This computer is too old to run this program.<br><br>"
            "Please try a much newer computer.<br><br>"
            "(Reason: SSE4.2 is required to run this program.)"
        );
        return false;
    }

    if (!CPU_CAPABILITY_NATIVE.OK_13_Haswell){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br>";
        str += "(i.e. Increased error rate. Fail to reliably detect shinies, etc...)<br><br>";
        str += "(Reason: Computers that lack AVX2 are likely to be too old and too slow to handle this program.)<br><br>";
        str += "Recommendation: Use a more powerful computer.";
        box.warning(nullptr, "Warning", str);
    }

    ProcessorSpecs specs = get_processor_specs();

    if (specs.threads < 4){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br>";
        str += "(i.e. Increased error rate. Fail to reliably detect shinies, etc...)<br><br>";
        str += "(Reason: ";
        str += QString::number(specs.threads);
        str += " vcores is a weak CPU.)<br><br>";
        str += "Recommendation: Use a more powerful computer.";
        box.warning(nullptr, "Warning", str);
    }

    if (specs.base_frequency < 2'000'000'000){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br>";
        str += "(i.e. Increased error rate. Fail to reliably detect shinies, etc...)<br><br>";
        str += "(Reason: Base frequency measured at ";
        str += QString::fromStdString(tostr_fixed(specs.base_frequency / 1000000000., 3));
        str += " GHz which is very slow.)<br><br>";
        str += "Recommendation: Use a more powerful computer.";
        box.warning(nullptr, "Warning", str);
    }

#if (defined _WIN32) || (defined __linux)
    size_t vcores = specs.threads > specs.cores
        ? specs.threads - specs.cores
        : 0;
    double efreq = (specs.cores + 0.2 * vcores) * specs.base_frequency;
    if (efreq < 8'000'000'000){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br>";
        str += "(i.e. Increased error rate. Fail to reliably detect shinies, etc...)<br><br>";
        str += "CPU: " + QString::fromStdString(specs.name) + "<br>";
        str += "Cores: " + QString::number(specs.cores) + "<br>";
        str += "Threads: " + QString::number(specs.threads) + "<br>";
        str += "Sockets: " + QString::number(specs.sockets) + "<br>";
        str += "Numa Nodes: " + QString::number(specs.numa_nodes) + "<br>";
        str += "Base Frequency: " + QString::fromStdString(tostr_fixed(specs.base_frequency / 1000000000., 3)) + " GHz<br>";
        str += "<br>";
        str += "(p-cores + 0.2 * v-cores) * base-frequency = ";
        str += QString::fromStdString(tostr_fixed(efreq / 1000000000., 3));
        str += " GHz<br><br>";
        str += "Recommendation: Use a more powerful computer.";
        box.warning(nullptr, "Warning", str);
    }
#endif

    return true;
}



}
