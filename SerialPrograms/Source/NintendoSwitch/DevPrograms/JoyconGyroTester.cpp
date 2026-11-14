/*  Joycon Gyro Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include <stdexcept>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "JoyconGyroTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;

namespace{

constexpr double MAX_ACCEL = std::numeric_limits<int32_t>::max() / Internal::GYRO_ACCEL_TRANSPORT_SCALE;
constexpr double MAX_GYRO = std::numeric_limits<int32_t>::max() / Internal::GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE;

Milliseconds validate_phases(const std::vector<JoyconGyroPhase>& phases){
    if (phases.empty()){
        throw std::invalid_argument("Add at least one motion phase.");
    }
    Milliseconds total = 0ms;
    for (const JoyconGyroPhase& phase : phases){
        if (phase.duration <= 0ms || phase.duration > Milliseconds::max() - total){
            throw std::invalid_argument("Motion phase durations must be positive and their total must fit in milliseconds.");
        }
        if (!is_finite(phase.target)){
            throw std::invalid_argument("Motion values must be finite.");
        }
        total += phase.duration;
    }
    return total;
}

}

GyroMotion make_joycon_gyro_motion(
    const std::vector<JoyconGyroPhase>& phases,
    bool return_to_start,
    Cancellable* cancellable
){
    const Milliseconds total = validate_phases(phases);
    const GyroFunction forward = [phases](Milliseconds elapsed){
        GyroState previous{};
        for (const JoyconGyroPhase& phase : phases){
            if (elapsed <= phase.duration){
                return interpolate(previous, phase.target, (double)elapsed.count() / phase.duration.count());
            }
            elapsed -= phase.duration;
            previous = phase.target;
        }
        return previous;
    };

    GyroMotion motion = prepare_gyro_motion(cancellable, total, forward);
    if (return_to_start){
        motion.append_inverse_rotation(cancellable);
    }
    return motion;
}


JoyconGyroPhaseRow::JoyconGyroPhaseRow(EditableTableOption& parent_table)
    : JoyconGyroPhaseRow(parent_table, "Phase", {150ms, {}})
{}

JoyconGyroPhaseRow::JoyconGyroPhaseRow(
    EditableTableOption& parent_table, std::string phase_name, JoyconGyroPhase phase
)
    : EditableTableRow(parent_table)
    , name(false, LockMode::LOCK_WHILE_RUNNING, std::move(phase_name), "Phase name")
    , duration(LockMode::LOCK_WHILE_RUNNING, 1ms, std::to_string(phase.duration.count()))
    , accel_x(LockMode::LOCK_WHILE_RUNNING, phase.target.acceleration_g.x, -MAX_ACCEL, MAX_ACCEL)
    , accel_y(LockMode::LOCK_WHILE_RUNNING, phase.target.acceleration_g.y, -MAX_ACCEL, MAX_ACCEL)
    , accel_z(LockMode::LOCK_WHILE_RUNNING, phase.target.acceleration_g.z, -MAX_ACCEL, MAX_ACCEL)
    , gyro_x(LockMode::LOCK_WHILE_RUNNING, phase.target.angular_velocity_dps.x, -MAX_GYRO, MAX_GYRO)
    , gyro_y(LockMode::LOCK_WHILE_RUNNING, phase.target.angular_velocity_dps.y, -MAX_GYRO, MAX_GYRO)
    , gyro_z(LockMode::LOCK_WHILE_RUNNING, phase.target.angular_velocity_dps.z, -MAX_GYRO, MAX_GYRO)
{
    PA_ADD_OPTION(name);
    PA_ADD_OPTION(duration);
    PA_ADD_OPTION(accel_x);
    PA_ADD_OPTION(accel_y);
    PA_ADD_OPTION(accel_z);
    PA_ADD_OPTION(gyro_x);
    PA_ADD_OPTION(gyro_y);
    PA_ADD_OPTION(gyro_z);
}

std::unique_ptr<EditableTableRow> JoyconGyroPhaseRow::clone() const{
    auto copy = std::make_unique<JoyconGyroPhaseRow>(parent(), (std::string)name, snapshot());
    copy->duration.set(duration.current_text());
    return copy;
}

JoyconGyroPhase JoyconGyroPhaseRow::snapshot() const{
    return {duration.get(), {{accel_x, accel_y, accel_z}, {gyro_x, gyro_y, gyro_z}}};
}

JoyconGyroPhaseTable::JoyconGyroPhaseTable()
    : EditableTableOption_t<JoyconGyroPhaseRow>(
        "<b>Motion phases:</b><br>"
        "Start at zero acceleration and angular velocity. Each row linearly ramps from the previous row's "
        "target to this row's target over its duration. Repeat a target in the next row to hold it. "
        "The host samples the trajectory every 15ms, including the final endpoint; shorter phases may fall between samples.<br>"
        "<b>Canonical axes:</b> +X toward the triggers, +Y toward the controller's left when looking at its controls, "
        "+Z out through the controls. Positive rotation follows the right-hand rule. "
        "Acceleration is in g; angular velocity is in degrees per second, not an angle. "
        "Gravity is not added automatically. The API handles the selected Joy-Con's axis conversion.<br>"
        "Defaults reproduce the right Joy-Con throw. Add, clone, or delete rows to build another motion.",
        LockMode::LOCK_WHILE_RUNNING
    )
{
    set_default(make_defaults());
    restore_defaults();
}

std::vector<std::string> JoyconGyroPhaseTable::make_header() const{
    return {"Phase", "Duration (ms)", "Accel X (g)", "Accel Y (g)", "Accel Z (g)",
        "Gyro X (°/s)", "Gyro Y (°/s)", "Gyro Z (°/s)"};
}

std::string JoyconGyroPhaseTable::check_validity() const{
    std::string error = EditableTableOption::check_validity();
    if (!error.empty()){
        return error;
    }
    try{
        validate_phases(snapshot<JoyconGyroPhase>());
    }catch (const std::invalid_argument& e){
        return e.what();
    }
    return {};
}

std::vector<std::unique_ptr<EditableTableRow>> JoyconGyroPhaseTable::make_defaults(){
    // The original throw was tuned in mode-2 report coordinates. Firmware maps
    // sensor (X, Y, Z) to mode-2 (Y, X, Z), and the right Joy-Con API maps
    // canonical (X, Y, Z) to sensor (X, -Y, -Z). Thus each old report vector
    // (x, y, z) is expressed here as canonical (y, -x, -z).
    std::vector<std::unique_ptr<EditableTableRow>> rows;
    rows.emplace_back(std::make_unique<JoyconGyroPhaseRow>(*this, "Wind-up", JoyconGyroPhase{
        150ms, {{0.3, 5.0, 0.8}, {8.400269239399, 168.005384787974, -8.400269239399}},
    }));
    rows.emplace_back(std::make_unique<JoyconGyroPhaseRow>(*this, "Release", JoyconGyroPhase{
        240ms, {{1.5, -8.0, -4.0}, {-35.001121830828, -630.020192954902, -42.001346196993}},
    }));
    rows.emplace_back(std::make_unique<JoyconGyroPhaseRow>(*this, "Reinforce release", JoyconGyroPhase{
        150ms, {{1.0, -8.0, -3.0}, {-21.000673098497, -700.022436616558, -21.000673098497}},
    }));
    rows.emplace_back(std::make_unique<JoyconGyroPhaseRow>(*this, "Settle", JoyconGyroPhase{180ms, {}}));
    return rows;
}


JoyconGyroTester_Descriptor::JoyconGyroTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:JoyconGyroTester",
        "Nintendo Switch", "Joycon Gyro Tester",
        "",
        "Build a motion trajectory using acceleration in g and angular velocity in degrees per second.",
        PanelQualityStanding::DEVELOPER,
        ProgramControllerClass::SpecializedController,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

JoyconGyroTester::JoyconGyroTester()
    : A_HOLD(
        "<b>Initial A press:</b><br>Hold A before the motion. Set to 0 to skip the press.",
        LockMode::LOCK_WHILE_RUNNING, "64 ms"
    )
    , START_DELAY(
        "<b>Delay before motion:</b><br>Wait after releasing A (or before motion when the press is skipped).",
        LockMode::LOCK_WHILE_RUNNING, "400 ms"
    )
    , RETURN_TO_START(
        "<b>Reverse rotation after motion:</b><br>Replay the sampled angular velocities in reverse order with opposite signs "
        "and zero acceleration. This undoes the commanded rotation; it does not reset or calibrate the controller. "
        "Turning this off leaves the final orientation in place. Zero angular velocity alone holds orientation.",
        LockMode::LOCK_WHILE_RUNNING, true
    )
{
    PA_ADD_OPTION(A_HOLD);
    PA_ADD_OPTION(START_DELAY);
    PA_ADD_OPTION(PHASES);
    PA_ADD_OPTION(RETURN_TO_START);
}

JoyconGyroTester::~JoyconGyroTester() = default;

void JoyconGyroTester::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconController& controller = env.console.controller<JoyconController>();
    controller.check_gyro_motion_supported();
    const std::string error = PHASES.check_validity();
    if (!error.empty()){
        throw UserSetupError(env.logger(), error);
    }
    const GyroMotion motion = make_joycon_gyro_motion(PHASES.snapshot<JoyconGyroPhase>(), RETURN_TO_START, &scope);

    controller.wait_for_all(&scope);
    const Milliseconds a_hold = A_HOLD;
    if (a_hold > 0ms){
        controller.issue_buttons(&scope, 0ms, a_hold, 0ms, BUTTON_A);
        controller.wait_for_all(&scope);
    }
    controller.issue_nop(&scope, START_DELAY);

    controller.issue_gyro_motion(&scope, motion);
    controller.wait_for_all(&scope);
}


}
}
