/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
//#include <QSystemTrayIcon>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/FillGeometry.h"
#include "CommonFramework/Inference/AnomalyDetector.h"
#include "CommonFramework/Inference/ColorClustering.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "CommonFramework/OCR/StringNormalization.h"
#include "CommonFramework/OCR/TextMatcher.h"
#include "CommonFramework/OCR/LargeDictionaryMatcher.h"
#include "Pokemon/Pokemon_SpeciesDatabase.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinySparkleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleDialogDetector.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "Pokemon/Pokemon_Notification.h"
#include "TestProgram.h"

#include <fstream>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


TestProgram_Descriptor::TestProgram_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Test Program",
        "",
        "Test Program",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



TestProgram::TestProgram(const TestProgram_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>OCR Language:</b>",
//        m_iv_checker_reader.languages()
#if 1
        {
//            Language::None,
            Language::English,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Japanese,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        false
#endif
    )
    , DROPDOWN(
        "<b>Test Dropdown:</b>",
        {
            "case 0",
            "case 1",
            "case 2",
            "case 3",
        }, 0
    )
    , STRING_SELECT(
        "<b>Test Select:</b>",
        {
            "Bulbasaur",
            "Charmander",
            "Charmeleon",
            "Charizard",
            "Squirtle",
            "Chikorita",
        }, 0
    )
    , FILTER(true, false)
{
    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&DROPDOWN, "DROPDOWN");
    m_options.emplace_back(&STRING_SELECT, "STRING_SELECT");
    m_options.emplace_back(&FILTER, "FILTER");
}



void read(std::map<std::string, std::map<Language, QString>>& database, Language language){
    std::string code = language_data(language).code;
    QString path = PERSISTENT_SETTINGS().resource_path +
        "Pokemon/PokemonNameOCR-baseline/PokemonOCR-" +
        code.c_str() + ".json";
    QJsonObject obj = read_json_file(path).object();
    for (auto iter = obj.begin(); iter != obj.end(); ++iter){
        database[iter.key().toUtf8().data()][language] = iter.value().toArray()[0].toString();
    }
}



void TestProgram::program(SingleSwitchProgramEnvironment& env){
    using namespace OCR;
    using namespace Pokemon;

//    BotBase& botbase = env.console;
//    VideoFeed& feed = env.console;

    BattleDialogDetector detector(env.console);

#if 0
    {
        ReceivePokemonDetector receive_detector(env.console);
        ShinySparkleDetector shiny_detector(
            env.console, env.logger(),
            InferenceBox(0.1, 0.01, 0.8, 0.77),
            0.7
        );
        AsyncVisualInferenceSession inference(env, env.console);
        inference += receive_detector;
        inference += shiny_detector;

        env.wait(std::chrono::seconds(30));

        shiny_detector.results();
    }
#endif




#if 0
    send_encounter_notification(
        nullptr,
        "Test Program",
        nullptr,
        ShinyType::SQUARE_SHINY
    );
#endif

#if 0
    std::map<std::string, std::map<Language, QString>> database;
    for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
        Language language = (Language)c;
        cout << language_data(language).code << endl;
        read(database, language);
    }

    QString str;
    str += "{\r\n";
    for (const auto& item : NATIONAL_DEX_SLUGS()){
        auto iter = database.find(item);
        if (iter == database.end()){
            PA_THROW_StringException("Missing slug.");
        }
        const std::map<Language, QString>& species = iter->second;

        str += "    \"";
        str += item.c_str();
        str += "\": {\r\n";

        for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
            Language language = (Language)c;
            auto iter1 = species.find(language);
            if (iter1 == species.end()){
                continue;
            }
            str += "        \"";
            str += language_data(language).code.c_str();
            str += "\": \"";
            str += iter1->second;
            if (c + 1 < (size_t)Language::EndOfList){
                str += "\",\r\n";
            }else{
                str += "\"\r\n";
            }
        }

        str += "    },\r\n";
    }
    str += "}";

    QFile file("PokemonNameTranslations.json");
    file.open(QIODevice::WriteOnly);
    std::string data = "\xef\xbb\xbf";
    data += str.toUtf8().data();
    file.write(data.c_str(), data.size());
#endif



//    DiscordWebHook::send_message(true, "test message", QJsonArray(), &env.logger());

//    DiscordWebHook::send_message(true, "hello world", QJsonArray());
//    send_encounter_notification(m_descriptor.display_name(), {}, ShinyType::SHINY);


//    cout << levenshtein_distance_substring("asdf", "a") << endl;
//    cout << random_match_probability(10, 10, 1. / 3) << endl;


//    env.log("asdf\nqwer");

//    QSystemTrayIcon icon;
//    icon.show();
//    icon.showMessage("title", "asdf");


#if 0
    QImage image(
        PERSISTENT_SETTINGS().training_data +
//        "PokemonNameOCR/PokemonNameOCR (Kim-SwShPokedex-0)/kor/clefairy-20210618-201635.png"
//        "PokemonNameOCR/PokemonNameOCR (Kim-SwShPokedex-0)/kor/cleffa-20210618-202023.png"
//        "PokemonNameOCR/PokemonNameOCR (Kim-SwShPokedex-1)/chi_tra/frillish-20210619-023228.png"
        "PokemonNameOCR/PokemonNameOCR (Kim-SwShPokedex-0)/deu/cleffa-20210618-203904.png"
//        "IVCheckerOCR/IVCheckerOCR (Kim-0)/eng/Decent-20210619-200037b.png"
    );

    BrightnessHistogram histogram(image);
    cout << histogram.dump() << endl;

//    OCR::binary_filter_black_text(image);
    make_OCR_filter(image).apply(image);

    image.save("test.png");

    QString text = OCR::ocr_read(LANGUAGE, image);
    env.log("OCR Read: " + text);
#endif



//    OCR::LargeDictionaryMatcher name_matcher("Pokemon/PokemonNameOCR/PokemonOCR");



//    OCR::LargeDatabaseOCR database(PERSISTENT_SETTINGS().resource_path + "Pokemon/PokemonNameOCR/PokemonOCR-eng.json");

//    database.write("text.json");


//    cout << levenshtein_distance("asdfqwer", "asdfqwer") << endl;
//    cout << levenshtein_distance("asdfqwer", "asddfqwe") << endl;

//    ReceivePokemonDetector detector(feed);
//    detector.receive_is_over(feed.snapshot());

//    generate_names_file();


#if 0
    QString text = "asdf";
    env.log(text);

    text = OCR::normalize(text);
    env.log(text);
#endif

#if 0
    text = text.normalized(QString::NormalizationForm_KD);
//    text = OCR::remove_non_alphanumeric(text);
    env.log(text);

    for (QChar ch : text){
        cout << ch.unicode() << endl;
    }
#endif


#if 0
    IVCheckerReaderScope reader(m_iv_checker_reader, feed, LANGUAGE);

    IVCheckerReader::Results results = reader.read(&env.logger(), feed.snapshot());

    cout << IVCheckerReader::enum_to_token(results.hp) << endl;
    cout << IVCheckerReader::enum_to_token(results.attack) << endl;
    cout << IVCheckerReader::enum_to_token(results.defense) << endl;
    cout << IVCheckerReader::enum_to_token(results.spatk) << endl;
    cout << IVCheckerReader::enum_to_token(results.spdef) << endl;
    cout << IVCheckerReader::enum_to_token(results.speed) << endl;
#endif



//    QChar ch(0x4ED6);
//    cout << ch.isLetterOrNumber() << endl;


#if 0
    InferenceBoxScope box0(env.console, InferenceBox(0.75, 0.531 + 0 * 0.1115, 0.18, 0.059));
    InferenceBoxScope box1(env.console, InferenceBox(0.75, 0.531 + 1 * 0.1115, 0.18, 0.059));
    InferenceBoxScope box2(env.console, InferenceBox(0.75, 0.531 + 2 * 0.1115, 0.18, 0.059));
    InferenceBoxScope box3(env.console, InferenceBox(0.75, 0.531 + 3 * 0.1115, 0.18, 0.059));


//    InferenceBoxScope box(env.console, InferenceBox(0.76, 0.04, 0.15, 0.044));

//    QImage frame = feed.snapshot();
//    frame = extract_box(frame, box);

//    QString str = TextInference::ocr_read(frame, LANGUAGE);
//    cout << str.toUtf8().data() << endl;

//    env.log(str);


#endif
    env.wait(std::chrono::seconds(60));


}







}
}




