/*  Hardware Validation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <thread>
#include <QMessageBox>
#include "Common/Cpp/PrettyPrint.h"
#include "Environment.h"
#include "HardwareValidation.h"

namespace PokemonAutomation{


bool check_hardware(){
    CPU_x86_Features features;
    if (!features.HW_SSE42){
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

    if (!features.HW_AVX2){
        QMessageBox box;
        box.warning(
            nullptr,
            "Warning",
            "This computer may not be powerful enough to run this program.<br><br>"
            "You can continue, but the program may not work correctly.<br><br>"
            "(Reason: Computers that lack AVX2 are likely to be too old and too slow to handle this program.)"
        );
    }

    int vcores = std::thread::hardware_concurrency();
    if (vcores < 4){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br><br>";
        str += "(Reason: ";
        str += QString::number(vcores);
        str += " vcores is a weak CPU.)";
        box.warning(nullptr, "Warning", str);
    }

    uint64_t rdtsc_freq = x86_rdtsc_ticks_per_sec();
    if (rdtsc_freq < 1'500'000'000){
        QMessageBox box;
        QString str;
        str += "This computer may not be powerful enough to run this program.<br><br>";
        str += "You can continue, but the program may not work correctly.<br><br>";
        str += "(Reason: Base frequency measured at ";
        str += QString::fromStdString(tostr_fixed(rdtsc_freq / 1000000000., 3));
        str += " GHz which is very slow.)";
        box.warning(nullptr, "Warning", str);
    }

    return true;
}





}
