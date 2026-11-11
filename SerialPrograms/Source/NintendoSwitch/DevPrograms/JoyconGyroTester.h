/*  Joycon Gyro Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_JoyconGyroTester_H
#define PokemonAutomation_NintendoSwitch_JoyconGyroTester_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Gyro.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
class UnitTestDatabase;
namespace NintendoSwitch{


struct JoyconGyroPhase{
    Milliseconds duration;
    GyroState target;
};

struct JoyconGyroMotion{
    Milliseconds duration;
    GyroFunction function;
};

// Build the forward trajectory and, optionally, its sampled inverse rotation.
std::vector<JoyconGyroMotion> make_joycon_gyro_motion(
    const std::vector<JoyconGyroPhase>& phases,
    bool return_to_start
);

void add_tests_JoyconGyroTester(UnitTestDatabase& database);

class JoyconGyroPhaseRow : public EditableTableRow{
public:
    JoyconGyroPhaseRow(EditableTableOption& parent_table);
    JoyconGyroPhaseRow(EditableTableOption& parent_table, std::string name, JoyconGyroPhase phase);
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    JoyconGyroPhase snapshot() const;

    StringCell name;
    MillisecondsCell duration;
    FloatingPointCell accel_x;
    FloatingPointCell accel_y;
    FloatingPointCell accel_z;
    FloatingPointCell gyro_x;
    FloatingPointCell gyro_y;
    FloatingPointCell gyro_z;
};

class JoyconGyroPhaseTable : public EditableTableOption_t<JoyconGyroPhaseRow>{
public:
    JoyconGyroPhaseTable();
    virtual std::vector<std::string> make_header() const override;
    virtual std::string check_validity() const override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};


class JoyconGyroTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    JoyconGyroTester_Descriptor();
};


class JoyconGyroTester : public SingleSwitchProgramInstance{
public:
    using Descriptor = JoyconGyroTester_Descriptor;

    ~JoyconGyroTester();
    JoyconGyroTester();

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    MillisecondsOption A_HOLD;
    MillisecondsOption START_DELAY;
    JoyconGyroPhaseTable PHASES;
    BooleanCheckBoxOption RETURN_TO_START;
};


}
}
#endif
