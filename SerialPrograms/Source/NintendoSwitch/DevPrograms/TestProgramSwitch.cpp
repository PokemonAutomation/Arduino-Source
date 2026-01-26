/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "TestProgramSwitch.h"

//#include <immintrin.h>
#include <QApplication>
#include <QClipboard>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IvJudgeReader.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Inference/NintendoSwitch_CheckOnlineDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardCodeEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Inference/Battles/PokemonSV_PostCatchDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Programs/Trading/PokemonSV_TradeRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSV/Inference/PokemonSV_PokePortalDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_AreaZeroPlatform.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "PokemonSV/Programs/PokemonSV_AreaZero.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonSV/Inference/PokemonSV_BagDetector.h"
#include <filesystem>
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSV/Inference/PokemonSV_StatHexagonReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMaterialDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
//#include "CommonFramework/Environment/SystemSleep.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h"
#include "PokemonSV/Programs/Farming/PokemonSV_AuctionFarmer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
//#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMenuDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "CommonTools/Images/ImageFilter.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_CloseGameDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_US.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_24h.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_StartBattleYellowBar.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSide.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathMap.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "PokemonLGPE/Inference/Battles/PokemonLGPE_BattleArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonMovesReader.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_RunFromBattleDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapIconDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "Common/Cpp/Options/CheckboxDropdownDatabase.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.tpp"
//#include "Integrations/PybindSwitchController.h"
#include "Common/PABotBase2/PABotBase2_ConnectionDebug.h"
#include "Common/PABotBase2/PABotBase2_PacketSender.h"
#include "Common/PABotBase2/PABotBase2_StreamCoalescer.h"
#include "Common/Cpp/StreamConnections/StreamInterface.h"
#include "Common/Cpp/StreamConnections/StreamConnection.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/CRC32/pabb_CRC32.h"
#include "Common/PABotBase2/PABotBase2_PacketParser.h"
#include "Common/Cpp/StreamConnections/ReliableStreamConnection.h"
#include "Common/PABotBase2/PABotbase2_ReliableStreamConnection.h"
#include "Common/Cpp/StreamConnections/MockDevice.h"
#include "ML/Inference/ML_PaddleOCRPipeline.h"
#include "CommonTools/OCR/OCR_RawPaddleOCR.h"



#include <QPixmap>
#include <QVideoFrame>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;

using namespace PokemonAutomation::Kernels;
using namespace PokemonAutomation::Kernels::Waterfill;




namespace PokemonAutomation{










}



namespace PokemonAutomation{
namespace NintendoSwitch{


const CheckboxDropdownDatabase<Button>& ProController_Button_Database();
const EnumDropdownDatabase<DpadPosition>& ProController_Dpad_Database();




TestProgram_Descriptor::TestProgram_Descriptor()
    : MultiSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Nintendo Switch", "Test Program (Switch)",
        "",
        "Test Program (Switch)",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        1, 4, 1
    )
{}


TestProgram::~TestProgram(){
    BUTTON1.remove_listener(*this);
    BUTTON0.remove_listener(*this);
}
TestProgram::TestProgram()
    : BUTTON0(
        "Button Text 0", 0, 16
    )
    , BUTTON1(
        "<b>Button Label:</b><br><font color=\"red\">asdfasdfasdf</font>",
        "Button Text 1", 0, 16
    )
    , LANGUAGE(
        "<b>OCR Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , IMAGE_PATH(false, "Path to image for testing", LockMode::UNLOCK_WHILE_RUNNING, "default.png", "default.png")
    , FLOAT("Float option:", LockMode::UNLOCK_WHILE_RUNNING, 0) 
    , STATIC_TEXT("Test text...")
    , BOX("Box", LockMode::UNLOCK_WHILE_RUNNING, 0, 0, 1, 1)
    , BUTTONS("Buttons", ProController_Button_Database(), LockMode::UNLOCK_WHILE_RUNNING, BUTTON_NONE)
    , DPAD(ProController_Dpad_Database(), LockMode::UNLOCK_WHILE_RUNNING, DPAD_NONE)
    , COMMANDS(
        "Command Schedule:",
        {
            ControllerClass::NintendoSwitch_ProController,
            ControllerClass::NintendoSwitch_LeftJoycon,
            ControllerClass::NintendoSwitch_RightJoycon,
        }
    )
    , NOTIFICATION_TEST("Test", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_TEST,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BUTTON0);
    PA_ADD_OPTION(BUTTON1);
    PA_ADD_OPTION(LANGUAGE);
//    PA_ADD_OPTION(CONSOLE_MODEL);
    PA_ADD_OPTION(IMAGE_PATH);
    PA_ADD_OPTION(FLOAT);
    PA_ADD_OPTION(STATIC_TEXT);
    PA_ADD_OPTION(BOX);
    PA_ADD_OPTION(BUTTONS);
    PA_ADD_OPTION(DPAD);
    PA_ADD_OPTION(COMMANDS);
//    PA_ADD_OPTION(battle_AI);
    PA_ADD_OPTION(NOTIFICATIONS);
    BUTTON0.add_listener(*this);
    BUTTON1.add_listener(*this);
}


//using namespace Kernels;
using namespace Kernels::Waterfill;



void TestProgram::on_press(){
    global_logger_tagged().log("Button Pressed");
//    BUTTON.set_enabled(false);
    BUTTON0.set_text("Button Pressed");
    BUTTON1.set_text("Button Pressed");
}




#if 0
class TealDialogMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    TealDialogMatcher() : WaterfillTemplateMatcher(
        "PokemonLZA/DialogBox/DialogBoxTitleGreenLine-Template.png", Color(180,200,70), Color(200, 220, 115), 50
    ) {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static DialogTitleGreenLineMatcher matcher;
        return matcher;
    }
};
#endif





class LogSender : public StreamSender{
public:
    virtual size_t send(const void* data, size_t bytes) override{
//        cout << PABotBase2::dump_packet((const pabb2_PacketHeader*)data) << endl;
        cout << "Sending: ";
        pabb2_PacketHeader_print((const pabb2_PacketHeader*)data, false);
        fflush(stdout);
        return bytes;
    }
};

void fp_LogSender(void* context, const void* data, size_t bytes){
    ((LogSender*)context)->send(data, bytes);
}

#if 0
std::string dump(const pabb2_PacketSender& sender){
    std::string str;
    str += "---------------\n";
    str += "Slot Head: " + std::to_string(sender.slot_head) + "\n";
    str += "Slot Tail: " + std::to_string(sender.slot_tail) + "\n";
    str += "Buffer Head: " + std::to_string(sender.buffer_head) + "\n";
    str += "Buffer Tail: " + std::to_string(sender.buffer_tail) + "\n";
    for (uint8_t seqnum = sender.slot_head; seqnum != sender.slot_tail; seqnum++){
        size_t offset = ~sender.offsets[seqnum & PABB2_ConnectionSender_SLOTS_MASK];
        str += "Offset = " + std::to_string(offset) + "\n";
        str += PABotBase2::dump_packet((const pabb2_PacketHeader*)(sender.buffer + offset));
        str += "\n";
    }
    return str;
}
#endif

struct DataPacket : pabb2_PacketHeaderData{
    char data[256];

    void set(uint8_t seqnum, uint16_t stream_offset, const char* str){
        const size_t OVERHEAD = sizeof(pabb2_PacketHeaderData) + sizeof(uint32_t);
        const size_t MAX_SIZE = 256 - OVERHEAD;

        size_t size = strlen(str);
        if (size >= MAX_SIZE){
            cout << "Data is too large." << endl;
            size = MAX_SIZE;
        }

        this->magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        this->seqnum = seqnum;
        this->packet_bytes = (uint8_t)(size + OVERHEAD);
        this->opcode = PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA;
        this->stream_offset = stream_offset;
        memcpy(data, str, size);
    }
};


#if 0
pabb2_StreamCoalescer* coalescer;


class MockConnection : public StreamConnection{
public:
    MockConnection()
        : m_thread([this]{ thread_body(); })
    {}
    ~MockConnection(){
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_stopping = true;
        }
        m_cv.notify_all();
        m_thread.join();
    }

    virtual size_t send(const void* data, size_t bytes) override{
        const pabb2_PacketHeader* packet = (const pabb2_PacketHeader*)data;
#if 0
        cout << "Sending: ";
        pabb2_PacketHeader_print(packet, false);
        fflush(stdout);
#endif

        WallClock now = current_time();

        struct{
            pabb2_PacketHeader header;
            uint8_t crc[sizeof(uint32_t)];
        } response;
        response.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
        response.header.seqnum = packet->seqnum;
        response.header.packet_bytes = sizeof(response);
        response.header.opcode = PABB2_CONNECTION_OPCODE_RET;
        pabb_crc32_write_to_message(&response, sizeof(response));

        std::lock_guard<std::mutex> lg(m_lock);

        if (packet->opcode == PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA){
            uint8_t stream_size = packet->packet_bytes - sizeof(pabb2_PacketHeaderData) - sizeof(uint32_t);
            for (uint8_t c = 0; c < stream_size; c++){
                char expected = '0' + m_offset % 10;
                char actual = ((const char*)packet)[sizeof(pabb2_PacketHeaderData) + c];
                if (expected != actual){
                    cout << "Mismatch at: " << m_offset << ", expected = " << expected << ", actual = " << actual << endl;

                    pabb2_StreamCoalescer_print(coalescer, true);

                    system("pause");
                }
                m_offset++;
            }

        }



//        cout << "enqueuing" << endl;
        m_send_schedule.insert({
            now + 500ms,
            std::string((char*)&response, (char*)&response + sizeof(response))
        });
        m_cv.notify_all();

        return bytes;
    }


private:
    void thread_body(){
        while (!m_stopping){
            std::unique_lock<std::mutex> lg(m_lock);

            while (!m_send_schedule.empty()){
                auto iter = m_send_schedule.begin();
                if (current_time() < iter->first){
                    m_cv.wait_until(lg, iter->first);
                    continue;
                }
                std::string& packet = iter->second;
#if 0
                cout << "Receiving: ";
                pabb2_PacketHeader_print((const pabb2_PacketHeader*)packet.data(), false);
                fflush(stdout);
#endif
                on_recv(packet.data(), packet.size());
                m_send_schedule.erase(iter);
            }

            m_cv.wait(lg);
        }
    }


private:
    std::multimap<WallClock, std::string> m_send_schedule;

    uint64_t m_offset = 0;

    std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_stopping = false;
    Thread m_thread;
};
#endif







void TestProgram::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
    using namespace Kernels::Waterfill;
    using namespace OCR;
    using namespace NintendoSwitch;
    using namespace Pokemon;
//    using namespace PokemonSwSh;
//    using namespace PokemonBDSP;
//    using namespace PokemonLA;
//    using namespace PokemonSV;
    using namespace PokemonLZA;

    [[maybe_unused]] Logger& logger = env.logger();
    [[maybe_unused]] ConsoleHandle& console = env.consoles[0];
//    [[maybe_unused]] BotBase& botbase = env.consoles[0];
    [[maybe_unused]] VideoFeed& feed = env.consoles[0];
    [[maybe_unused]] VideoOverlay& overlay = env.consoles[0];
    ProControllerContext context(scope, console.controller<ProController>());
    // JoyconContext context(scope, console.controller<JoyconController>());
    VideoOverlaySet overlays(overlay);



    UpdateMenuWatcher update_menu(console, COLOR_PURPLE);
    CheckOnlineWatcher check_online(COLOR_CYAN);
    update_menu.make_overlays(overlays);
    check_online.make_overlays(overlays);




#if 0
    {
        MockDevice device;

        ReliableStreamConnection connection(
            logger,
            device,
            1s
        );

        connection.send_request(PABB2_CONNECTION_OPCODE_ASK_VERSION);
        connection.send_request(PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE);
        connection.send_request(PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS);
        connection.send_request(PABB2_CONNECTION_OPCODE_ASK_RESET);


        context.wait_for(60s);
    }
#endif




#if 0
    CloseGameWatcher close_game(console);
    close_game.make_overlays(overlays);

    auto snapshot = feed.snapshot();
    cout << close_game.detect(snapshot) << endl;
#endif


#if 0
    MockConnection unreliable_connection;
    {
        ReliableStreamConnection connection(logger, unreliable_connection, 100s);

        coalescer = &connection.m_stream_coalescer;


        cout << connection.send_request(0x20) << endl;
//        cout << connection.send_request(0x21) << endl;

        while (true){
            scope.throw_if_cancelled();
            cout << "Sent = " << connection.send("0123456789", 10) << endl;
            pabb2_PacketSender_print(&connection.m_reliable_sender, true);
            scope.wait_for(50ms);
        }


        scope.wait_for(10s);
        cout << "================ End Test ================" << endl;
    }
#endif

#if 0
    DataPacket packet;
    pabb2_StreamCoalescer coalescer;
    pabb2_StreamCoalescer_init(&coalescer);

    coalescer.stream_head = 32;
    coalescer.stream_tail = 32;

    packet.set(3, 32, "asdf");
    cout << pabb2_StreamCoalescer_push_stream(&coalescer, &packet) << endl;

    pabb2_StreamCoalescer_push_packet(&coalescer, 1);
    pabb2_StreamCoalescer_push_packet(&coalescer, 2);
    pabb2_StreamCoalescer_print(&coalescer, true);

    pabb2_StreamCoalescer_push_packet(&coalescer, 0);
    pabb2_StreamCoalescer_print(&coalescer, true);

    packet.set(5, 60, "qwersdfg");
    cout << pabb2_StreamCoalescer_push_stream(&coalescer, &packet) << endl;
    pabb2_StreamCoalescer_print(&coalescer, true);

    packet.set(4, 36, "01234567890123456789abcd");
    cout << pabb2_StreamCoalescer_push_stream(&coalescer, &packet) << endl;

    pabb2_StreamCoalescer_print(&coalescer, true);


    char buffer[100] = {};

    cout << "read = " << pabb2_StreamCoalescer_read(&coalescer, buffer, 1) << endl;
    cout << buffer << endl;

    pabb2_StreamCoalescer_print(&coalescer, true);

    cout << "read = " << pabb2_StreamCoalescer_read(&coalescer, buffer, 10) << endl;
    cout << buffer << endl;

    pabb2_StreamCoalescer_print(&coalescer, true);

    cout << "read = " << pabb2_StreamCoalescer_read(&coalescer, buffer, 100) << endl;
    cout << buffer << endl;

    pabb2_StreamCoalescer_print(&coalescer, true);
#endif


#if 0
    LogSender sender;
    pabb2_PacketSender queue;
    pabb2_PacketSender_init(&queue, fp_LogSender, &sender);

    cout << pabb2_PacketSender_send_stream(&queue, "asdf", 4) << endl;
    cout << pabb2_PacketSender_send_stream(&queue, "qwer", 4) << endl;
    cout << pabb2_PacketSender_send_stream(&queue, "zxcv", 4) << endl;
    pabb2_PacketSender_remove(&queue, 0);

    cout << pabb2_PacketSender_send_stream(&queue, "01234567890123456789", 20) << endl;
//    cout << pabb2_PacketSender_send_stream(&queue, "sdfg", 4) << endl;
//    cout << pabb2_PacketSender_send_stream(&queue, "xcvb", 4) << endl;

    pabb2_PacketSender_print(&queue, true);

    pabb2_PacketSender_remove(&queue, 0);
    cout << pabb2_PacketSender_send_stream(&queue, "sdfgh", 5) << endl;
    cout << pabb2_PacketSender_send_stream(&queue, "xcvb", 4) << endl;

    pabb2_PacketSender_print(&queue, true);
#endif


#if 0
    LogSender sender;
    pabb2_PacketSender queue;
    pabb2_PacketSender_init(&queue);

    pabb2_PacketHeader* packet;

    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 1;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 2;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 3;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    packet = pabb2_PacketSender_reserve_packet(&queue, 15);
    if (packet){
        packet->opcode = 4;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 5;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    cout << dump(queue) << endl;

#if 1
    pabb2_PacketSender_remove(&queue, 0);
    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 6;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }

    cout << dump(queue) << endl;
#endif

    packet = pabb2_PacketSender_reserve_packet(&queue, 16);
    if (packet){
        packet->opcode = 7;
        pabb2_PacketSender_commit_packet(&queue, packet);
        sender.send(packet, packet->packet_bytes);
    }else{
        cout << "Reserve Failed" << endl;
    }


#endif



#if 0
    LogSender sender;
    PABotBase2::RequestQueue queue(logger, sender, 16, 64);

    try{
        cout << queue.send_str("asdfqwe") << endl;
        cout << queue.send_str("wer") << endl;
    }catch (...){
        cout << "Caught Exception" << endl;
    }

    cout << "Dumping queue... 0" << endl;
    cout << queue.dump_queue(true) << endl;

#if 0
    {
        queue.report_acked(0);
        cout << "Dumping queue... 3" << endl;
        cout << queue.dump_queue(true) << endl;
    }
    {
        queue.report_acked(4);
        cout << "Dumping queue... 2" << endl;
        cout << queue.dump_queue(true) << endl;
    }
#endif
    {
        queue.report_acked(7);
        cout << "Dumping queue... 1" << endl;
        cout << queue.dump_queue(true) << endl;
    }
#endif




#if 0
    {
        PybindSwitchProController controller("COM12");
        controller.push_button(1000, 200, 800, BUTTON_B);
        controller.wait_for_all_requests();
    }
#endif


#if 0

    // ImageRGB32 image1(IMAGE_PATH);
    auto image1 = feed.snapshot();
    ImageViewRGB32 cropped = extract_box_reference(image1, ImageFloatBox{BOX.x(), BOX.y(), BOX.width(), BOX.height()});

    // auto snapshot = feed.snapshot();
    std::string text = OCR::paddle_ocr_read(LANGUAGE, cropped);
    cout << text << endl;

    

#endif

#if 0
    std::string move_results = "Move Effectiveness:";
    move_results += "\n- Top: " + std::string("asdf");
    move_results += "\n- Left: " + std::string("asdf");
    move_results += "\n- Right: " + std::string("asdf");
    move_results += "\n- Bottom: " + std::string("asdf");
    console.log(move_results);
#endif

//    close_game_from_home(console, context);


#if 0
    auto snapshot = feed.snapshot();

    BoxWatcher box;
    box.make_overlays(overlays);
    cout << box.detect(snapshot) << endl;
#endif



#if 0
    auto snapshot = feed.snapshot();
    cout << snapshot->width() << " x " << snapshot->height() << endl;

    RunFromBattleDetector detector(COLOR_RED, &overlay);
    cout << detector.detect(snapshot) << endl;
#endif

#if 0
close_game_from_home(console, context);
// ssf_issue_scroll(context, DPAD_DOWN, 24ms);
#endif  

#if 0
    // auto snapshot = feed.snapshot();
    // CloseGameDetector detector(console);
    // cout << detector.detect(snapshot) << endl;
    CloseGameWatcher watcher(console);

    int ret = wait_until(console, context, Seconds(10), {watcher});

    if (ret == 0){
        console.log("CloseGameWatcher detected.");
    }
    
#endif    

#if 0
    ImageRGB32 image1("itemprinter.png");
    PokemonSV::ItemPrinterMaterialDetector detector(COLOR_RED, Language::ChineseTraditional);
    // detector.detect_material_name(console, image1, context, 0);
    
    // detector.detect_material_name(console, image1, context, 2);
    detector.detect_material_name(console, image1, context, 1);



#endif

#if 0
    save_game_to_menu(console, context);
#endif



//    MoveEffectivenessSymbolMatcher::NoEffect();

#if 0
    auto snapshot = feed.snapshot();
    ImageViewRGB32 cropped = extract_box_reference(snapshot, ImageFloatBox(0.309013, 0.719466, 0.418455, 0.015267));
    ImageStats stats = image_stats(cropped);
    cout << stats.average << stats.stddev << endl;
#endif


#if 0
    MapIconDetector detector0(COLOR_RED, MapIconType::PokemonCenter, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector1(COLOR_RED, MapIconType::Building, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector2(COLOR_RED, MapIconType::BuildingFlyable, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector3(COLOR_RED, MapIconType::CafeFlyable, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector4(COLOR_RED, MapIconType::WildZone, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector5(COLOR_RED, MapIconType::WildZoneFlyable, {0, 0, 1, 1}, &overlay);
    MapIconDetector detector6(COLOR_RED, MapIconType::BattleZone, {0, 0, 1, 1}, &overlay);


    auto snapshot = feed.snapshot();
    detector0.detect(snapshot);
//    detector1.detect(snapshot);
//    detector2.detect(snapshot);
//    detector3.detect(snapshot);
//    detector4.detect(snapshot);
//    detector5.detect(snapshot);
//    detector6.detect(snapshot);
#endif



#if 0

    auto snapshot = feed.snapshot();

    ItemReceiveDetector detector(COLOR_RED, &overlay);
    cout << detector.detect(snapshot) << endl;
#endif

    
#if 0
    ImageRGB32 image1("swap-moves.png");
    PokemonSV::PokemonMovesReader reader(Language::Korean);
    std::string top_move = reader.read_move(env.logger(), image1, 2);
    env.log("Current top move: " + top_move);

    ImageRGB32 image2(IMAGE_PATH);
    ImageFloatBox box{0.396429, 0.506356, 0.069048, 0.059322};
    PokemonSV::MenuOption session(console, context, Language::Korean);
    session.read_option(extract_box_reference(image2, box));
#endif

#if 0
    while (true){
        sit_on_bench(console, context);
    }
#endif


#if 0
    while (true){
        ssf_press_button(context, BUTTON_ZL, 160ms, 800ms, 200ms);
        ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
        pbf_press_button(context, BUTTON_A, 80ms, 24ms);
        pbf_press_button(context, BUTTON_Y, 80ms, 24ms);
        pbf_press_button(context, BUTTON_X, 80ms, 24ms);
        pbf_press_button(context, BUTTON_B, 80ms, 24ms);
    }
#endif


#if 0
//    ImageRGB32 image("Screenshots/screenshot-20251025-153957561163.png");

    auto screen = feed.snapshot();

//    PokemonLZA::FlatWhiteDialogDetector detector(COLOR_RED, &overlay);
    PokemonLZA::BlueDialogDetector detector(COLOR_RED, &overlay);
//    PokemonLZA::ButtonDetector detector(COLOR_RED, ButtonType::ButtonA, {0.526718, 0.535922, 0.033806, 0.100971},  &overlay);

    cout << detector.detect(screen) << endl;
#endif


#if 0
    auto screen = feed.snapshot();

    ImageFloatBox box(0.473282, 0.572816, 0.058888, 0.100971);

    overlays.add(COLOR_RED, box);

    ImageViewRGB32 cropped = extract_box_reference(screen, box);

    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            cropped,
            0xffc04030, 0xffff8f6f
        );

        ImageRGB32 masked = cropped.copy();
        filter_by_mask(matrix, masked, Color(0xff000000), true);
        masked.save("MoveEffectivenessNormal.png");
    }
#endif

#if 0
    ImageRGB32 image("WildZoneFlyable.png");

    image = filter_rgb32_range(image, 0xffffffff, 0xffffffff, Color(0), true);
    image = filter_rgb32_range(image, 0xff000000 | (237 << 16) | (28 << 8) | 36, 0xff000000 | (237 << 16) | (28 << 8) | 36, Color(0), true);

    image.save("temp2.png");
#endif

#if 0
//    ImageRGB32 image("Screenshots/screenshot-20251012-174842583706.png");

    auto screen = feed.snapshot();

//    ImageFloatBox box(0.483097, 0.469903, 0.041439, 0.067961);
    ImageFloatBox box(0.405671, 0.310680, 0.056707, 0.099029);

    overlays.add(COLOR_RED, box);

    ImageViewRGB32 cropped = extract_box_reference(screen, box);

    //  Remove background.
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            cropped,
            0xff000000, 0xff4f4f7f
        );
        matrix.invert();

        cout << matrix.dump() << endl;

        auto session = Waterfill::make_WaterfillSession(matrix);
        auto iter = session->make_iterator(10);
        WaterfillObject object;
        iter->find_next(object, true);
        ImageRGB32 masked = extract_box_reference(cropped, object).copy();
        filter_by_mask(object.packed_matrix(), masked, Color(0x00000000), false);
        masked.save("temp.png");
    }


#if 0
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        cropped,
        0xffc0c0c0, 0xffffffff
    );

    cout << matrix.dump() << endl;

    ImageRGB32 tmp = cropped.copy();
//    filter_by_mask(matrix, tmp, Color(0x00000000), true);

    {
    }

    auto session = Waterfill::make_WaterfillSession(matrix);
    auto iter = session->make_iterator(10);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        break;
    }
    cout << "area = " << object.area << endl;

    tmp.save("DialogBoxWhiteArrow-Template.png");
//    extract_box_reference(tmp, object).save("SelectionArrow.png");




#endif
#endif


//    reset_game_to_gamemenu(console, context);

#if 0
    while (true){
        go_home(console, context);
        PokemonLZA::reset_game_from_home(env, console, context, true);
    }


    cout << "done" << endl;
#endif


#if 0
    ImageRGB32 image("screenshot-20221121-070043212515.png");

    WhiteButtonDetector next_button(WhiteButton::ButtonA, {0.8, 0.9, 0.2, 0.1}, COLOR_RED);
    cout << next_button.detect(image) << endl;
#endif



#if 0
    auto image = feed.snapshot();
    ImageViewRGB32 cropped = extract_box_reference(image, ImageFloatBox{0.500, 0.555, 0.310, 0.070});
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_euclidean(cropped, 0xff757f9c, 100);
//    cout << matrix.dump() << endl;

    matrix.invert();
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;
    while (iter->find_next(object, false)){
//        extract_box_reference(image, object).save("Arrow-Template.png");
        cout << object.area << endl;
    }
#endif


#if 0
    auto image = feed.snapshot();

    ImageViewRGB32 cropped = extract_box_reference(image, ImageFloatBox{0.500, 0.555, 0.310, 0.070});
//    ImageRGB32 filtered = filter_rgb32_range(cropped, 0xffc00000, 0xffffffff, Color(0x00000000), true);
    size_t pixels;
    ImageRGB32 filtered = filter_rgb32_euclidean(pixels, cropped, 0xff757f9c, 100, Color(0x00000000), true);
    cout << "pixels = " << pixels << endl;
    filtered.save("test.png");
#endif


#if 0
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff808000, 0xffffff80);

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        extract_box_reference(image, object).save("Arrow-Template.png");
    }
#endif





#if 0
//    ImageRGB32 image("SV-BattleMenu.png");
//    ImageRGB32 image("SV-Hair.png");
//    image = image.scale_to(1920, 1080);


//    extract_box_reference(image, ImageFloatBox({0.7, 0.6, 0.2, 0.1})).save("tmp.png");

//    ImageFloatBox box(0.5, 0.5, 0.4, 0.5);
    ImageFloatBox box(0.0, 0.0, 1.0, 1.0);


    VideoOverlaySet set(overlay);
    WhiteButtonFinder white_button_detector0(WhiteButton::ButtonA, overlay, box);
    WhiteButtonFinder white_button_detector1(WhiteButton::ButtonB, overlay, box);
    WhiteButtonFinder white_button_detector2(WhiteButton::ButtonY, overlay, box);
    WhiteButtonFinder white_button_detector3(WhiteButton::ButtonMinus, overlay, box);
    DialogArrowFinder dialog_arrow_detector(overlay, box);
    GradientArrowFinder gradient_arrow_detector(overlay, box);
    dialog_arrow_detector.make_overlays(set);
    gradient_arrow_detector.make_overlays(set);
    BattleMenuFinder battle_menu;
    battle_menu.make_overlays(set);

    while (true){
        scope.wait_for(std::chrono::milliseconds(50));
        VideoSnapshot snapshot = feed.snapshot();

        white_button_detector0.process_frame(snapshot, current_time());
        white_button_detector1.process_frame(snapshot, current_time());
        white_button_detector2.process_frame(snapshot, current_time());
        white_button_detector3.process_frame(snapshot, current_time());
        dialog_arrow_detector.process_frame(snapshot, current_time());
        gradient_arrow_detector.process_frame(snapshot, current_time());
        battle_menu.process_frame(snapshot, current_time());
    }
#endif






    scope.wait_for(std::chrono::seconds(60));


}




#if 0
struct RaidShinyStar{
    double alpha;
    WaterfillObject object;
};


class RaidShinyStarDetector : public VisualInferenceCallback{
    static constexpr double ALPHA_THRESHOLD = 1.0;

public:
    RaidShinyStarDetector(VideoOverlay& overlay)
        : VisualInferenceCallback("RaidShinyStarDetector")
        , m_overlay(overlay)
        , m_box(overlay, {0.5, 0.1, 0.4, 0.7})
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    std::vector<RaidShinyStar> process_frame(const ImageViewRGB32& frame);

private:
    double test_object(const ImageViewRGB32& image, const WaterfillObject& object);


private:
    VideoOverlay& m_overlay;
    OverlayBoxScope m_box;
    std::deque<OverlayBoxScope> m_stars;
};


void RaidShinyStarDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool RaidShinyStarDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    process_frame(frame);
    return false;
}

double RaidShinyStarDetector::test_object(const ImageViewRGB32& image, const WaterfillObject& object){
    double aspect_ratio = object.aspect_ratio();
    if (aspect_ratio < 0.9 || aspect_ratio > 1.1){
        return 0;
    }

    double area_ratio = object.area_ratio();
    if (area_ratio < 0.5 || area_ratio > 0.8){
        return 0;
    }

    //  Check that center of gravity is centered.
    double center_of_gravity_x = object.center_of_gravity_x();
    double center_of_gravity_y = object.center_of_gravity_y();
    double center_x = object.min_x + object.width() * 0.5;
    double center_y = object.min_y + object.height() * 0.5;

    double center_shift_x = center_x - center_of_gravity_x;
    double center_shift_y = center_y - center_of_gravity_y;
    center_shift_x *= center_shift_x;
    center_shift_y *= center_shift_y;

    double max_x_sqr = object.width() * 0.1;
    double max_y_sqr = object.height() * 0.1;
    max_x_sqr *= max_x_sqr;
    max_y_sqr *= max_y_sqr;

    if (center_shift_x > max_x_sqr){
        return 0;
    }
    if (center_shift_y > max_y_sqr){
        return 0;
    }

    return 1.0;
}


std::vector<RaidShinyStar> RaidShinyStarDetector::process_frame(const ImageViewRGB32& frame){

    ImageViewRGB32 cropped = extract_box_reference(frame, m_box);

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(cropped, {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},

        {0xff804040, 0xffffffff},
        {0xff905050, 0xffffffff},
        {0xffa06060, 0xffffffff},

        {0xff408040, 0xffffffff},
        {0xff509050, 0xffffffff},
        {0xff60a060, 0xffffffff},

        {0xff404080, 0xffffffff},
        {0xff505090, 0xffffffff},
        {0xff6060a0, 0xffffffff},
    });

//    std::vector<RaidShinyStar>
    std::vector<RaidShinyStar> stars;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    WaterfillObject object;
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        auto iter = session->make_iterator(10);
        while (iter->find_next(object, false)){
            double alpha = test_object(cropped, object);
            if (alpha >= ALPHA_THRESHOLD){
                stars.emplace_back(RaidShinyStar{alpha, object});
            }
        }
    }






    //  Redraw the boxes.
    m_stars.clear();
    for (const RaidShinyStar& star : stars){
        m_stars.emplace_back(m_overlay, translate_to_parent(frame, m_box, star.object), COLOR_BLUE);
    }
    return stars;
}

#endif




}
}




