/*  Pokemon Automation Bot Base Client
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include <ctime>
#include <string>
#include <iostream>

#if 0
#include <Windows.h>
#include "Common/Clientside/AsyncDispatcher.h"
using std::cout;
using std::endl;
#endif

namespace PokemonAutomation{

//  Device Logger
void device_logger              (const std::string& device_name = "");

//  Sample Programs
void program_TurboA             (const std::string& device_name = "");
void program_ClothingBuyer      (const std::string& device_name = "");
void program_BallThrower        (const std::string& device_name = "");
void program_BeamReset          (const std::string& device_name = "");
void program_FriendDelete       (const std::string& device_name = "");
void program_DateSpam_WattFarmer(const std::string& device_name = "");

void sandbox(const std::string& device_name);

}


int main(){
    using namespace PokemonAutomation;

#if 0
    try{
        AsyncDispatcher runner;
        auto task0 = runner.dispatch([]{
            cout << "asdf" << endl;
            Sleep(500);
//            throw 10;
        });
        auto task1 = runner.dispatch([]{
            cout << "qwer" << endl;
            Sleep(500);
//            throw 20;
        });
        Sleep(1000);

        task0->wait();
        task1->wait();

    }catch (int x){
        cout << x << endl;
    }catch (...){
        cout << "rethrow" << endl;
    }
#endif



    try{
        //  By default, this program runs the logging program.
        device_logger();

        //  You can also run programs directly from the computer.
//        program_TurboA();
//        program_ClothingBuyer();
//        program_BallThrower();
//        program_BeamReset();
//        program_FriendDelete();
//        program_DateSpam_WattFarmer();

//        sandbox("");
    }catch (const char* str){
        std::cout << std::endl;
        std::cout << "Exception: " << str << std::endl;
    }catch (const std::string& str){
        std::cout << std::endl;
        std::cout << "Exception: " << str << std::endl;
    }catch (...){
        std::cout << std::endl;
        std::cout << "Unknown Exception" << std::endl;
    }
    std::cout << std::endl;

#if _WIN32
    system("pause");
#endif
}
