/*  Nintendo Switch Gyro Tests
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <cmath>
#include <limits>
#include <stdexcept>
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
        if (!Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 0ms, function).empty()){
            return "0ms gyro motion generated a sample.";
        }
        if (!Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, -1ms, function).empty()){
            return "Negative gyro motion generated a sample.";
        }
        if (calls != 0){
            return "A non-positive gyro motion invoked its callback.";
        }

        const auto one = Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 1ms, function);
        if (one.size() != 1 || one[0].duration != 1ms || one[0].state.acceleration_g.x != 1){
            return "1ms gyro motion was not sampled at its right edge.";
        }
        const auto fifteen = Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 15ms, function);
        if (fifteen.size() != 1 || fifteen[0].duration != 15ms || fifteen[0].state.acceleration_g.x != 15){
            return "15ms gyro motion was not sampled correctly.";
        }
        const auto sixteen = Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 16ms, function);
        if (sixteen.size() != 2 || sixteen[0].duration != 15ms || sixteen[1].duration != 1ms
            || sixteen[0].state.acceleration_g.x != 15 || sixteen[1].state.acceleration_g.x != 16){
            return "16ms gyro motion does not preserve the short final interval.";
        }
        const auto thirty_one = Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 31ms, function);
        if (thirty_one.size() != 3 || thirty_one[0].duration != 15ms || thirty_one[1].duration != 15ms
            || thirty_one[2].duration != 1ms || thirty_one[2].state.acceleration_g.x != 31){
            return "31ms gyro motion does not have right-edge samples at 15, 30, and 31ms.";
        }

        bool threw = false;
        try{
            Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 31ms,
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
                    Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 1ms,
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
            Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 16ms,
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
        }catch (const std::invalid_argument&){
            range_threw = true;
        }
        if (!range_threw || range_calls != 2){
            return "A trajectory outside the firmware transport range was not rejected during sampling.";
        }

        range_threw = false;
        try{
            Internal::sample_gyro_motion(nullptr, ControllerClass::NintendoSwitch_LeftJoycon, 1ms,
                [](Milliseconds){
                    GyroState state{};
                    state.angular_velocity_dps.x =
                        static_cast<double>(std::numeric_limits<int32_t>::min())
                            / Internal::GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE - 1.0;
                    return state;
                }
            );
        }catch (const std::invalid_argument&){
            range_threw = true;
        }
        if (!range_threw){
            return "An angular velocity outside the firmware transport range was accepted.";
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
    database.add<Test_GyroTransform>();
    database.add<Test_GyroQuaternion>();
}


}
}
