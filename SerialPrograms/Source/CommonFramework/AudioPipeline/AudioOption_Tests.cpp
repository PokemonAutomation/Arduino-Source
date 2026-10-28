/*  Audio Pipeline - AudioOption JSON Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Gate tests for the descriptor-based AudioOption JSON layout
 *  (SourceType + the type-keyed params node, mirroring
 *  VideoSourceOption). Pure data tests: no audio I/O.
 *
 */

#include <string>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/AudioPipeline/AudioSources/AudioSource_Null.h"
#include "CommonFramework/AudioPipeline/AudioSources/AudioSource_File.h"
#include "CommonFramework/AudioPipeline/AudioSources/AudioSource_Device.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "AudioOption_Tests.h"

namespace PokemonAutomation{

namespace{
std::string json_string_or_missing(const std::string* str){
    if (str == nullptr){
        return "\"<missing>\"";
    }
    return "\"" + *str + "\"";
}





class AudioOptionJsonNoneTest : public UnitTest{
public:
    AudioOptionJsonNoneTest()
        : UnitTest("AudioOption - JSON_None")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        AudioOption option;
        JsonValue json = option.to_json();
        const JsonObject* obj = json.to_object();
        if (obj == nullptr){
            return "to_json() did not return an object.";
        }
        const std::string* type = obj->get_string("SourceType");
        if (type == nullptr || *type != "None"){
            return "SourceType: got " + json_string_or_missing(type)
                + ", expected \"None\".";
        }
        if (option.input_source()->type != AudioSourceType::None){
            return "Default option should hold a null-type source.";
        }

        AudioOption loaded;
        loaded.load_json(json);
        if (loaded.input_source()->type != AudioSourceType::None){
            return "Loaded option should have a null-type source.";
        }
        return true;
    }
};



class AudioOptionJsonFileTest : public UnitTest{
public:
    AudioOptionJsonFileTest()
        : UnitTest("AudioOption - JSON_File")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const std::string path = "/tmp/test_audio_source.mp3";
        AudioOption option;
        option.set_input_source(std::make_shared<AudioSourceDescriptor_File>(path));
        JsonValue json = option.to_json();
        const JsonObject* obj = json.to_object();
        if (obj == nullptr){
            return "to_json() did not return an object.";
        }
        const std::string* type = obj->get_string("SourceType");
        if (type == nullptr || *type != "Audio File"){
            return "SourceType: got " + json_string_or_missing(type)
                + ", expected \"Audio File\".";
        }
        const JsonValue* params = obj->get_value("Audio File");
        const std::string* saved_path = params ? params->to_string() : nullptr;
        if (saved_path == nullptr || *saved_path != path){
            return "\"Audio File\" params: got " + json_string_or_missing(saved_path)
                + ", expected " + path + ".";
        }

        AudioOption loaded;
        loaded.load_json(json);
        if (loaded.input_source()->type != AudioSourceType::File){
            return "Loaded option should have a file source.";
        }
        const std::string loaded_path =
            static_cast<const AudioSourceDescriptor_File&>(*loaded.input_source()).path();
        if (loaded_path != path){
            return "Loaded path: got " + loaded_path + ", expected " + path + ".";
        }
        return true;
    }
};



class AudioOptionJsonDeviceTest : public UnitTest{
public:
    AudioOptionJsonDeviceTest()
        : UnitTest("AudioOption - JSON_Device")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        //  AudioDeviceInfo drops names not in the live enumeration, so a
        //  device round-trip needs a real device.
        const std::vector<AudioDeviceInfo> devices = AudioDeviceInfo::all_input_devices();
        if (devices.empty()){
            return true;
        }
        AudioDeviceInfo device0 = devices[0];

        AudioOption option;
        option.set_input_source(
            std::make_shared<AudioSourceDescriptor_Device>(device0)
        );
        JsonValue json = option.to_json();
        const JsonObject* obj = json.to_object();
        if (obj == nullptr){
            return "to_json() did not return an object.";
        }
        const std::string* type = obj->get_string("SourceType");
        if (type == nullptr || *type != "Audio Device"){
            return "SourceType: got " + json_string_or_missing(type)
                + ", expected \"Audio Device\".";
        }
        const JsonValue* params = obj->get_value("Audio Device");
        const std::string* saved_name = params ? params->to_string() : nullptr;
        if (saved_name == nullptr || *saved_name != device0.device_name()){
            return "\"Audio Device\" params: got " + json_string_or_missing(saved_name)
                + ", expected " + device0.device_name() + ".";
        }

        AudioOption loaded;
        loaded.load_json(json);
        if (loaded.input_source()->type != AudioSourceType::Device){
            return "Loaded option should have a device source.";
        }
        const AudioDeviceInfo& device =
            static_cast<const AudioSourceDescriptor_Device&>(*loaded.input_source()).device_info();
        if (device.device_name() != device0.device_name()){
            return "Loaded device: got " + device.device_name()
                + ", expected " + device0.device_name() + ".";
        }

        //  The saved input format round-trips.
        {
            const AudioInputChoice choice =
                device0.supported_choices()[device0.preferred_choice_index()];
            JsonObject node;
            node["SourceType"] = "Audio Device";
            node["Audio Device"] = device0.device_name();
            node["InputFormat"] = choice.display_name();
            AudioOption loaded2;
            loaded2.load_json(JsonValue(std::move(node)));
            if (loaded2.input_choice() != choice){
                return "Loaded input format doesn't match the saved choice.";
            }
        }
        return true;
    }
};



}


void add_tests_AudioOption(UnitTestDatabase& database){
    database.add<AudioOptionJsonNoneTest>();
    database.add<AudioOptionJsonFileTest>();
    database.add<AudioOptionJsonDeviceTest>();
}



}
