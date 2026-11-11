/*  Nintendo Switch Gyro Tests
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <cmath>
#include <limits>
#include <stdexcept>
#include "NintendoSwitch_ControllerWithScheduler.h"
#include "NintendoSwitch_Gyro.h"
#include "NintendoSwitch_GyroQuaternion.h"
#include "NintendoSwitch_GyroTests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

namespace{

class Test_GyroSampling : public UnitTest{
public:
    Test_GyroSampling()
        : UnitTest("NintendoSwitch::Gyro - sampling")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        using namespace std::chrono_literals;
        size_t calls = 0;
        const GyroFunction function = [&calls](Milliseconds elapsed){
            ++calls;
            return GyroState{{(double)elapsed.count(), 0, 0}, {0, 0, 0}};
        };
        if (!prepare_gyro_motion(nullptr, 0ms, function).empty()){
            return "0ms gyro motion generated a sample.";
        }
        if (!prepare_gyro_motion(nullptr, -1ms, function).empty()){
            return "Negative gyro motion generated a sample.";
        }
        if (calls != 0){
            return "A non-positive gyro motion invoked its callback.";
        }

        const auto one = prepare_gyro_motion(nullptr, 1ms, function).samples();
        if (one.size() != 1 || one[0].duration != 1ms || one[0].state.acceleration_g.x != 1){
            return "1ms gyro motion was not sampled at its right edge.";
        }
        const auto fifteen = prepare_gyro_motion(nullptr, 15ms, function).samples();
        if (fifteen.size() != 1 || fifteen[0].duration != 15ms || fifteen[0].state.acceleration_g.x != 15){
            return "15ms gyro motion was not sampled correctly.";
        }
        const auto sixteen = prepare_gyro_motion(nullptr, 16ms, function).samples();
        if (sixteen.size() != 2 || sixteen[0].duration != 15ms || sixteen[1].duration != 1ms
            || sixteen[0].state.acceleration_g.x != 15 || sixteen[1].state.acceleration_g.x != 16){
            return "16ms gyro motion does not preserve the short final interval.";
        }
        const auto thirty_one = prepare_gyro_motion(nullptr, 31ms, function).samples();
        if (thirty_one.size() != 3 || thirty_one[0].duration != 15ms || thirty_one[1].duration != 15ms
            || thirty_one[2].duration != 1ms || thirty_one[2].state.acceleration_g.x != 31){
            return "31ms gyro motion does not have right-edge samples at 15, 30, and 31ms.";
        }

        bool threw = false;
        try{
            prepare_gyro_motion(nullptr, 31ms,
                [](Milliseconds elapsed){
                    if (elapsed == 30ms){
                        throw std::runtime_error("expected");
                    }
                    return GyroState{};
                }
            );
        }catch (const std::runtime_error&){
            threw = true;
        }
        if (!threw){
            return "A throwing gyro callback did not propagate its exception.";
        }

        const double invalid[] = {
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::infinity(),
        };
        for (double value : invalid){
            for (int field = 0; field < 6; ++field){
                bool invalid_threw = false;
                try{
                    prepare_gyro_motion(nullptr, 1ms,
                        [field, value](Milliseconds){
                            GyroState state{};
                            switch (field){
                            case 0: state.acceleration_g.x = value; break;
                            case 1: state.acceleration_g.y = value; break;
                            case 2: state.acceleration_g.z = value; break;
                            case 3: state.angular_velocity_dps.x = value; break;
                            case 4: state.angular_velocity_dps.y = value; break;
                            case 5: state.angular_velocity_dps.z = value; break;
                            }
                            return state;
                        }
                    );
                }catch (const std::invalid_argument&){
                    invalid_threw = true;
                }
                if (!invalid_threw){
                    return "A non-finite gyro motion field was accepted.";
                }
            }
        }

        size_t range_calls = 0;
        bool range_threw = false;
        try{
            const GyroMotion motion = prepare_gyro_motion(nullptr, 16ms,
                [&range_calls](Milliseconds elapsed){
                    ++range_calls;
                    GyroState state{};
                    if (elapsed == 16ms){
                        state.acceleration_g.z =
                            static_cast<double>(std::numeric_limits<int32_t>::max())
                                / Internal::GYRO_ACCEL_TRANSPORT_SCALE + 1.0;
                    }
                    return state;
                }
            );
            Internal::transform_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, motion);
        }catch (const std::invalid_argument&){
            range_threw = true;
        }
        if (!range_threw || range_calls != 2){
            return "A trajectory outside the firmware transport range was not rejected before scheduling.";
        }

        range_threw = false;
        try{
            const GyroMotion motion = prepare_gyro_motion(nullptr, 1ms,
                [](Milliseconds){
                    GyroState state{};
                    state.angular_velocity_dps.x =
                        static_cast<double>(std::numeric_limits<int32_t>::min())
                            / Internal::GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE - 1.0;
                    return state;
                }
            );
            Internal::transform_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, motion);
        }catch (const std::invalid_argument&){
            range_threw = true;
        }
        if (!range_threw){
            return "An angular velocity outside the firmware transport range was accepted.";
        }
        return true;
    }
};

class Test_GyroInverse : public UnitTest{
public:
    Test_GyroInverse()
        : UnitTest("NintendoSwitch::Gyro - prepared inverse")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        using namespace std::chrono_literals;
        GyroMotion empty;
        empty.append_inverse_rotation();
        if (!empty.empty() || empty.duration() != 0ms){
            return "An empty motion gained samples during inverse replay.";
        }
        for (Milliseconds duration : {1ms, 8ms, 15ms, 16ms, 31ms, 720ms}){
            size_t calls = 0;
            GyroMotion motion = prepare_gyro_motion(nullptr, duration, [&calls](Milliseconds elapsed){
                // Deliberately stateful: reevaluating this callback cannot undo
                // the original motion. Changing axes also exercises ordering.
                const double n = (double)++calls;
                return GyroState{{1, 2, 3}, {n * 10, (double)elapsed.count(), n * n}};
            });
            const auto forward = motion.samples();
            motion.append_inverse_rotation();
            if (calls != forward.size() || motion.samples().size() != 2 * calls
                || motion.duration() != duration * 2){
                return "Inverse replay resampled the callback or changed the total duration.";
            }
            for (size_t i = 0; i < forward.size(); ++i){
                const GyroMotionSample& original = forward[i];
                const GyroMotionSample& inverse = motion.samples()[motion.samples().size() - 1 - i];
                if (inverse.duration != original.duration
                    || inverse.state.acceleration_g.x != 0 || inverse.state.acceleration_g.y != 0
                    || inverse.state.acceleration_g.z != 0
                    || inverse.state.angular_velocity_dps.x != -original.state.angular_velocity_dps.x
                    || inverse.state.angular_velocity_dps.y != -original.state.angular_velocity_dps.y
                    || inverse.state.angular_velocity_dps.z != -original.state.angular_velocity_dps.z){
                    return "Inverse replay did not preserve the original sample boundaries and opposite rotation.";
                }
            }
            for (ControllerClass controller_class : {
                ControllerClass::NintendoSwitch_LeftJoycon, ControllerClass::NintendoSwitch_RightJoycon,
            }){
                GyroQuaternion orientation{};
                for (const auto& sample : Internal::transform_gyro_motion(nullptr, controller_class, motion)){
                    orientation = integrate_angular_velocity(orientation, sample.state.angular_velocity_dps, sample.duration);
                }
                if (std::fabs(orientation.x) > 1e-12 || std::fabs(orientation.y) > 1e-12
                    || std::fabs(orientation.z) > 1e-12 || std::fabs(orientation.w - 1) > 1e-12){
                    return "Prepared multi-axis motion and its inverse did not return to the starting orientation.";
                }
            }
        }

        CancellableHolder<CancellableScope> cancelled;
        cancelled.cancel(std::make_exception_ptr(std::runtime_error("expected cancellation")));
        GyroMotion motion = prepare_gyro_motion(nullptr, 31ms, [](Milliseconds){ return GyroState{}; });
        bool threw = false;
        try{
            motion.append_inverse_rotation(&cancelled);
        }catch (const std::runtime_error&){
            threw = true;
        }
        if (!threw || motion.duration() != 31ms || motion.samples().size() != 3){
            return "Cancelled inverse preparation modified the original motion.";
        }
        return true;
    }
};

class GyroRecordingController : public ControllerWithScheduler{
public:
    using ControllerWithScheduler::ControllerWithScheduler;
    struct Entry{
        WallDuration duration;
        SwitchControllerState state;
        bool has_gyro;
    };
    std::vector<Entry> entries;

private:
    virtual void execute_state(Cancellable*, const SuperscalarScheduler::ScheduleEntry& entry) override{
        SwitchControllerState state{};
        bool has_gyro = false;
        for (const auto& resource : entry.state){
            static_cast<const SwitchCommand&>(*resource).apply(state);
            has_gyro |= resource->id == (size_t)SwitchResource::GYRO;
        }
        entries.push_back({entry.duration, state, has_gyro});
    }
};

class Test_GyroScheduling : public UnitTest{
public:
    Test_GyroScheduling()
        : UnitTest("NintendoSwitch::Gyro - prepared scheduling")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        using namespace std::chrono_literals;
        RecursiveThrottler throttler;
        ThrottleScope suppress_logging(throttler);
        const GyroFunction function = [](Milliseconds elapsed){
            return GyroState{{1, 2, 3}, {4, (double)elapsed.count(), 6}};
        };
        GyroMotion motion = prepare_gyro_motion(nullptr, 31ms, function);
        motion.append_inverse_rotation();
        for (ControllerClass controller_class : {
            ControllerClass::NintendoSwitch_LeftJoycon, ControllerClass::NintendoSwitch_RightJoycon,
        }){
            GyroRecordingController controller(logger, throttler);
            controller.issue_buttons(nullptr, 0ms, 62ms, 0ms, BUTTON_A);
            controller.issue_gyro_motion(nullptr, controller_class, motion);
            if (controller.entries.size() != motion.samples().size()){
                return "Scheduling did not dispatch every prepared sample before returning.";
            }
            const double sign = controller_class == ControllerClass::NintendoSwitch_RightJoycon ? -1 : 1;
            for (size_t i = 0; i < motion.samples().size(); ++i){
                const auto& expected = motion.samples()[i];
                const auto& actual = controller.entries[i];
                if (!actual.has_gyro || actual.duration != expected.duration || actual.state.buttons != BUTTON_A
                    || actual.state.gyro.acceleration_g.x != expected.state.acceleration_g.x
                    || actual.state.gyro.acceleration_g.y != sign * expected.state.acceleration_g.y
                    || actual.state.gyro.acceleration_g.z != sign * expected.state.acceleration_g.z
                    || actual.state.gyro.angular_velocity_dps.x != expected.state.angular_velocity_dps.x
                    || actual.state.gyro.angular_velocity_dps.y != sign * expected.state.angular_velocity_dps.y
                    || actual.state.gyro.angular_velocity_dps.z != sign * expected.state.angular_velocity_dps.z){
                    return "Prepared motion lost button composition, timing, or canonical axis conversion.";
                }
            }
        }
        if (motion.samples()[0].state.acceleration_g.y != 2){
            return "Dispatch modified the reusable canonical motion.";
        }

        GyroRecordingController legacy(logger, throttler);
        GyroRecordingController prepared(logger, throttler);
        legacy.issue_gyro_motion(nullptr, ControllerClass::NintendoSwitch_RightJoycon, 31ms, function);
        prepared.issue_gyro_motion(nullptr, ControllerClass::NintendoSwitch_RightJoycon,
            prepare_gyro_motion(nullptr, 31ms, function));
        if (legacy.entries.size() != 3 || prepared.entries.size() != 3){
            return "Callback and prepared APIs produced different sample counts.";
        }
        for (size_t i = 0; i < 3; ++i){
            if (legacy.entries[i].duration != prepared.entries[i].duration
                || legacy.entries[i].state.gyro.angular_velocity_dps.y != prepared.entries[i].state.gyro.angular_velocity_dps.y){
                return "Callback and prepared APIs produced different schedules.";
            }
        }

        GyroRecordingController followed_by_wait(logger, throttler);
        followed_by_wait.issue_gyro_motion(nullptr, ControllerClass::NintendoSwitch_RightJoycon, 15ms, function);
        followed_by_wait.issue_nop(nullptr, 100ms);
        if (followed_by_wait.entries.size() != 2
            || !followed_by_wait.entries[0].has_gyro || followed_by_wait.entries[0].duration != 15ms
            || followed_by_wait.entries[1].has_gyro || followed_by_wait.entries[1].duration != 100ms){
            return "The wait after gyro motion did not get its full duration.";
        }

        prepared.issue_buttons(nullptr, 20ms, 20ms, 0ms, BUTTON_A);
        if (prepared.entries.size() != 4 || prepared.entries[2].state.buttons != BUTTON_NONE
            || prepared.entries[3].has_gyro || prepared.entries[3].state.buttons != BUTTON_A
            || prepared.entries[3].duration != 20ms){
            return "A button press after gyro motion did not start after the final sample.";
        }

        // Forward INT32_MIN is valid on X, but negating it is not encodable.
        // Detect even an invalid inverse before queueing any forward samples.
        GyroMotion invalid = prepare_gyro_motion(nullptr, 16ms, [](Milliseconds){
            return GyroState{{}, {std::numeric_limits<int32_t>::min() / Internal::GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE, 0, 0}};
        });
        invalid.append_inverse_rotation();
        GyroRecordingController rejected(logger, throttler);
        bool threw = false;
        try{
            rejected.issue_gyro_motion(nullptr, ControllerClass::NintendoSwitch_RightJoycon, invalid);
        }catch (const std::invalid_argument&){
            threw = true;
        }
        rejected.issue_barrier(nullptr);
        if (!threw || !rejected.entries.empty()){
            return "An invalid prepared motion changed scheduler state before validation finished.";
        }
        return true;
    }
};

class Test_GyroTransform : public UnitTest{
public:
    Test_GyroTransform()
        : UnitTest("NintendoSwitch::Gyro - canonical frame")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const GyroState input{{1, 2, 3}, {4, 5, 6}};
        for (ControllerClass controller_class : {
            ControllerClass::NintendoSwitch_LeftJoycon,
            ControllerClass::NintendoSwitch_ProController,
        }){
            const GyroState output = transform_gyro_state(controller_class, input);
            if (output.acceleration_g.x != 1 || output.acceleration_g.y != 2 || output.acceleration_g.z != 3
                || output.angular_velocity_dps.x != 4 || output.angular_velocity_dps.y != 5 || output.angular_velocity_dps.z != 6){
                return "An identity gyro frame transform changed an axis.";
            }
        }
        const GyroState right = transform_gyro_state(ControllerClass::NintendoSwitch_RightJoycon, input);
        if (right.acceleration_g.x != 1 || right.acceleration_g.y != -2 || right.acceleration_g.z != -3
            || right.angular_velocity_dps.x != 4 || right.angular_velocity_dps.y != -5 || right.angular_velocity_dps.z != -6){
            return "Right Joy-Con gyro frame transform is incorrect.";
        }
        const GyroState twice = transform_gyro_state(ControllerClass::NintendoSwitch_RightJoycon, right);
        if (twice.acceleration_g.x != input.acceleration_g.x || twice.acceleration_g.y != input.acceleration_g.y
            || twice.acceleration_g.z != input.acceleration_g.z || twice.angular_velocity_dps.x != input.angular_velocity_dps.x
            || twice.angular_velocity_dps.y != input.angular_velocity_dps.y || twice.angular_velocity_dps.z != input.angular_velocity_dps.z){
            return "Right Joy-Con gyro transform is not its own inverse.";
        }
        try{
            transform_gyro_state(ControllerClass::None, input);
        }catch (const std::invalid_argument&){
            return true;
        }
        return "Unsupported controller class did not reject gyro motion.";
    }
};

class Test_GyroQuaternion : public UnitTest{
public:
    Test_GyroQuaternion()
        : UnitTest("NintendoSwitch::Gyro - quaternion")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        using namespace std::chrono_literals;
        const GyroQuaternion identity{};
        const GyroQuaternion stationary = integrate_angular_velocity(identity, {}, 15ms);
        if (stationary.x != 0 || stationary.y != 0 || stationary.z != 0 || stationary.w != 1){
            return "Stationary gyro motion did not preserve identity.";
        }
        const GyroQuaternion quarter_turn = integrate_angular_velocity(identity, {90, 0, 0}, 1000ms);
        const double half_sqrt = std::sqrt(0.5);
        if (std::fabs(quarter_turn.x - half_sqrt) > 1e-12 || std::fabs(quarter_turn.w - half_sqrt) > 1e-12
            || std::fabs(quarter_turn.y) > 1e-12 || std::fabs(quarter_turn.z) > 1e-12){
            return "90dps X-axis quaternion integration is incorrect.";
        }
        GyroQuaternion chunked{};
        for (int i = 0; i < 1000 / 15; ++i){
            chunked = integrate_angular_velocity(chunked, {90, 0, 0}, 15ms);
        }
        chunked = integrate_angular_velocity(chunked, {90, 0, 0}, 10ms);
        if (std::fabs(chunked.x - quarter_turn.x) > 1e-12 || std::fabs(chunked.w - quarter_turn.w) > 1e-12){
            return "Chunked single-axis gyro integration drifted from one-step integration.";
        }
        const GyroQuaternion returned = integrate_angular_velocity(quarter_turn, {-90, 0, 0}, 1000ms);
        if (std::fabs(returned.x) > 1e-12 || std::fabs(returned.y) > 1e-12 || std::fabs(returned.z) > 1e-12
            || std::fabs(returned.w - 1) > 1e-12){
            return "Inverse gyro angular velocity did not return to identity.";
        }

        OemController_State0x30_GyroQuaternion packed{};
        pack_quaternion(packed, identity, 0);
        const uint8_t identity_golden[] = {
            0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
        };
        for (size_t i = 0; i < sizeof(identity_golden); ++i){
            if (packed.raw[i] != identity_golden[i]){
                return "Identity gyro quaternion packing no longer matches its mode-2 bytes.";
            }
        }
        OemController_State0x30_GyroQuaternion packed_half{};
        pack_quaternion(packed_half, {0.5, 0.5, 0.5, 0.5}, 0);
        const uint8_t half_golden[] = {
            0, 0, 0, 0, 0, 0, 2, 0, 128, 0, 0, 16, 0, 0, 0, 0, 0, 0,
            0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
        };
        for (size_t i = 0; i < sizeof(half_golden); ++i){
            if (packed_half.raw[i] != half_golden[i]){
                return "Nontrivial gyro quaternion packing no longer matches its mode-2 bytes.";
            }
        }
        OemController_State0x30_GyroQuaternion packed_wrapped{};
        pack_quaternion(packed_wrapped, identity, 0xfff);
        if (packed_wrapped.raw[33] != 128 || packed_wrapped.raw[34] != 255 || packed_wrapped.raw[35] != 15){
            return "Gyro quaternion timestamp does not wrap at 11 bits.";
        }
        return true;
    }
};

}

void add_tests_Gyro(UnitTestDatabase& database){
    database.add<Test_GyroSampling>();
    database.add<Test_GyroInverse>();
    database.add<Test_GyroScheduling>();
    database.add<Test_GyroTransform>();
    database.add<Test_GyroQuaternion>();
}


}
}
