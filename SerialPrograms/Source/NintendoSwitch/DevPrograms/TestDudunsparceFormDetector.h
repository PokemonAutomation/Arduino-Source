/*  Test Dudunsparce Form Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

 #ifndef PokemonAutomation_NintendoSwitch_TestDudunsparceFormDetector_H
 #define PokemonAutomation_NintendoSwitch_TestDudunsparceFormDetector_H
 
 #include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
 
 namespace PokemonAutomation{
 namespace NintendoSwitch{
 
 
 class TestDudunsparceFormDetector_Descriptor : public SingleSwitchProgramDescriptor{
 public:
     TestDudunsparceFormDetector_Descriptor();
 };
 
 
 class TestDudunsparceFormDetector : public SingleSwitchProgramInstance{
 public:
     TestDudunsparceFormDetector();
 
     virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
 
 private:
 };
 
 
 
 }
 }
 #endif // TESTPATHMAKER_H
 