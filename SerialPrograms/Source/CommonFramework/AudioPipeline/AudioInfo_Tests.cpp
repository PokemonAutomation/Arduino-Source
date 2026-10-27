/*  Audio Pipeline - AudioInfo / Input Choice Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Gate tests for the device-driven input format list
 *  (AudioInputChoice / input_choices). Pure data tests: no audio I/O.
 *
 */

#include <string>
#include <vector>
#include <QAudioFormat>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "AudioInfo_Tests.h"

namespace PokemonAutomation{

namespace{



//  Builds a QAudioFormat. The channel count is set before the channel
//  config, since Qt's setChannelCount() resets the config.
QAudioFormat make_q_format(
    int rate, int channels,
    QAudioFormat::ChannelConfig config, QAudioFormat::SampleFormat sample_format
){
    QAudioFormat format;
    format.setSampleRate(rate);
    format.setSampleFormat(sample_format);
    format.setChannelCount(channels);
    if (config != QAudioFormat::ChannelConfigUnknown){
        format.setChannelConfig(config);
    }
    return format;
}

AudioFormat make_format(
    int rate, int channels,
    QAudioFormat::ChannelConfig config = QAudioFormat::ChannelConfigUnknown,
    QAudioFormat::SampleFormat sample_format = QAudioFormat::SampleFormat::Int16
){
    return AudioFormat(make_q_format(rate, channels, config, sample_format));
}



class InputChoiceNamingTest : public UnitTest{
public:
    InputChoiceNamingTest()
        : UnitTest("AudioInfo - InputChoice_Naming")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        std::string error;
        const auto check_name = [](const std::string& what, const std::string& got, const std::string& want, std::string& err){
            if (got == want){
                return true;
            }
            err = what + ": got \"" + got + "\", expected \"" + want + "\".";
            return false;
        };

        //  Standard layouts.
        std::string got;
        got = AudioInputChoice(make_format(48000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono)).display_name();
        if (!check_name("1ch mono", got, "1 x 48,000 Hz (Mono)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(44100, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)).display_name();
        if (!check_name("2ch stereo 44.1k", got, "2 x 44,100 Hz (Stereo)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(96000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono)).display_name();
        if (!check_name("1ch mono 96k", got, "1 x 96,000 Hz (Mono)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(48000, 3, QAudioFormat::ChannelConfig::ChannelConfig2Dot1)).display_name();
        if (!check_name("2.1", got, "3 x 48,000 Hz (2.1)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(48000, 6, QAudioFormat::ChannelConfig::ChannelConfigSurround5Dot1)).display_name();
        if (!check_name("5.1", got, "6 x 48,000 Hz (5.1)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(48000, 8, QAudioFormat::ChannelConfig::ChannelConfigSurround7Dot1)).display_name();
        if (!check_name("7.1", got, "8 x 48,000 Hz (7.1)", error)){
            return error;
        }
        got = AudioInputChoice(make_format(192000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)).display_name();
        if (!check_name("comma thousands", got, "2 x 192,000 Hz (Stereo)", error)){
            return error;
        }

        //  Unknown config falls back to the standard layout name.
        got = AudioInputChoice(make_format(48000, 2, QAudioFormat::ChannelConfigUnknown)).display_name();
        if (!check_name("unknown config 2ch", got, "2 x 48,000 Hz (Stereo)", error)){
            return error;
        }

        //  Non-standard channel counts with no standard layout get no name.
        got = AudioInputChoice(make_format(48000, 9, QAudioFormat::ChannelConfigUnknown)).display_name();
        if (!check_name("9ch", got, "9 x 48,000 Hz", error)){
            return error;
        }

        //  The reinterpreted capture-card choices keep the preset strings.
        const AudioFormat physical = make_format(96000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono);
        const AudioFormat logical = make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo);
        got = AudioInputChoice::reinterpreted(AudioFormat(logical), AudioFormat(physical), false).display_name();
        if (!check_name("reinterpret L/R", got, "2 x 48,000 Hz (Interleaved Stereo L/R)", error)){
            return error;
        }
        got = AudioInputChoice::reinterpreted(AudioFormat(logical), AudioFormat(physical), true).display_name();
        if (!check_name("reinterpret R/L", got, "2 x 48,000 Hz (Interleaved Stereo R/L)", error)){
            return error;
        }

        //  An invalid choice has no name.
        if (!AudioInputChoice().display_name().empty()){
            return "Invalid choice should have an empty display name.";
        }

        return true;
    }
};



class InputChoiceFlagsTest : public UnitTest{
public:
    InputChoiceFlagsTest()
        : UnitTest("AudioInfo - InputChoice_Flags")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const AudioFormat format = make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo);

        //  Native choice: physical == format, no reinterpretation.
        AudioInputChoice native(format);
        if (!native.is_valid() || !native){
            return "Native choice should be valid.";
        }
        if (native.is_reinterpreted() || native.swap_channels()){
            return "Native choice should not be reinterpreted or swapped.";
        }
        if (native.physical_format().native_format() != format.native_format()){
            return "Native choice's physical format should equal its format.";
        }

        //  Reinterpreted choice: distinct shapes, swap flag honored.
        const AudioFormat physical = make_format(96000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono);
        for (bool swap : {false, true}){
            AudioInputChoice choice = AudioInputChoice::reinterpreted(AudioFormat(format), AudioFormat(physical), swap);
            if (!choice.is_reinterpreted()){
                return "Reinterpreted choice should report is_reinterpreted().";
            }
            if (choice.swap_channels() != swap){
                return "Reinterpreted choice's swap_channels() mismatch.";
            }
            if (choice.format().native_format() != format.native_format()
                || choice.physical_format().native_format() != physical.native_format()){
                return "Reinterpreted choice should carry the given format / physical format.";
            }
        }

        //  Invalid: the default-constructed choice.
        AudioInputChoice invalid;
        if (invalid.is_valid() || invalid){
            return "Default-constructed choice should be invalid.";
        }

        return true;
    }
};



class InputChoicesSortDedupeTest : public UnitTest{
public:
    InputChoicesSortDedupeTest()
        : UnitTest("AudioInfo - InputChoices_SortDedupe")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        //  Unordered input with a (rate, ch, config) duplicate in a
        //  different sample format.
        std::vector<AudioFormat> supported = {
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo),
            make_format(44100, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo),
            make_format(48000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono),
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo, QAudioFormat::SampleFormat::Float),
        };
        const AudioFormat preferred = make_format(
            48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo, QAudioFormat::SampleFormat::Float
        );

        AudioInputChoiceList list = input_choices(supported, preferred);

        if (list.choices.size() != 3){
            return "Expected 3 choices after dedup, got " + std::to_string(list.choices.size()) + ".";
        }
        //  Display order: channel count desc, then sample rate desc.
        if (list.choices[0].display_name() != "2 x 48,000 Hz (Stereo)"
            || list.choices[1].display_name() != "2 x 44,100 Hz (Stereo)"
            || list.choices[2].display_name() != "1 x 48,000 Hz (Mono)"){
            return "Choices are not sorted by channel count desc, then sample rate desc: ["
                + list.choices[0].display_name() + ", "
                + list.choices[1].display_name() + ", "
                + list.choices[2].display_name() + "].";
        }

        //  The higher quality sample format wins the duplicate collapse.
        if (list.choices[0].format().sample_format() != AudioSampleFormat::FLOAT32){
            return "float should beat int16 in the duplicate collapse.";
        }
        if (list.preferred_index != 0){
            return "preferred_index should point at the exact match (48k stereo float, index 0), got "
                + std::to_string(list.preferred_index) + ".";
        }

        //  No preferred match: the higher quality sample format still
        //  wins the collapse, and the fallback score picks the only entry.
        const AudioFormat preferred_elsewhere = make_format(44100, 1, QAudioFormat::ChannelConfig::ChannelConfigMono);
        AudioInputChoiceList list2 = input_choices({
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo),
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo, QAudioFormat::SampleFormat::Float),
        }, preferred_elsewhere);
        if (list2.choices.size() != 1){
            return "Expected 1 choice, got " + std::to_string(list2.choices.size()) + ".";
        }
        if (list2.choices[0].format().sample_format() != AudioSampleFormat::FLOAT32){
            return "float should beat int16 in the duplicate collapse regardless of the preferred format.";
        }
        if (list2.preferred_index != 0){
            return "preferred_index should fall back to the only entry without a match.";
        }

        //  No preferred match: preferred_index falls back to the highest
        //  quality score, not the first entry. 2ch x 48000 float
        //  (1.0 x 2.0 x sqrt(48000)) beats 8ch x 48000 float
        //  (1.0 x 1.875 x sqrt(48000)) and 1ch x 44100 int16
        //  (0.5 x 1.0 x sqrt(44100)).
        AudioInputChoiceList list3 = input_choices({
            make_format(44100, 1, QAudioFormat::ChannelConfig::ChannelConfigMono),
            make_format(48000, 8, QAudioFormat::ChannelConfig::ChannelConfigSurround7Dot1, QAudioFormat::SampleFormat::Float),
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo, QAudioFormat::SampleFormat::Float),
        }, make_format(32000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono));
        if (list3.choices.size() != 3){
            return "Expected 3 choices, got " + std::to_string(list3.choices.size()) + ".";
        }
        if (list3.choices[0].display_name() != "8 x 48,000 Hz (7.1)"
            || list3.choices[1].display_name() != "2 x 48,000 Hz (Stereo)"
            || list3.choices[2].display_name() != "1 x 44,100 Hz (Mono)"){
            return "Choices are not sorted by channel count desc, then sample rate desc: ["
                + list3.choices[0].display_name() + ", "
                + list3.choices[1].display_name() + ", "
                + list3.choices[2].display_name() + "].";
        }
        if (list3.preferred_index != 1){
            return "Without a preferred match, the highest-scoring choice should win (2ch x 48000 float, index 1), got "
                + std::to_string(list3.preferred_index) + ".";
        }

        return true;
    }
};



class InputChoicesCardQuirkTest : public UnitTest{
public:
    InputChoicesCardQuirkTest()
        : UnitTest("AudioInfo - InputChoices_CardQuirk")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const AudioFormat card = make_format(96000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono);

        //  The misreporting card: the only supported format is 1ch x 96000.
        AudioInputChoiceList list = input_choices({card}, card);
        if (list.choices.size() != 3){
            return "Expected 3 choices for a 1ch x 96000-only device, got "
                + std::to_string(list.choices.size()) + ".";
        }

        //  [0] The native choice.
        if (list.choices[0].is_reinterpreted()
            || list.choices[0].format().native_format() != card.native_format()){
            return "Choice 0 should be the native 1ch x 96000 entry.";
        }

        //  [1] / [2] The reinterpretations: the same physical stream read
        //  as 2ch x 48000 stereo, in the physical sample format, L/R
        //  first and R/L second.
        for (size_t i = 1; i <= 2; i++){
            const AudioInputChoice& choice = list.choices[i];
            if (!choice.is_reinterpreted() || choice.swap_channels() != (i == 2)){
                return "Choice " + std::to_string(i) + " should be reinterpreted"
                    + (i == 2 ? " with swap" : " without swap") + ".";
            }
            const QAudioFormat& logical = choice.format().native_format();
            if (logical.sampleRate() != 48000 || logical.channelCount() != 2
                || logical.channelConfig() != QAudioFormat::ChannelConfig::ChannelConfigStereo
                || logical.sampleFormat() != QAudioFormat::SampleFormat::Int16){
                return "Choice " + std::to_string(i) + " should be 2ch x 48000 stereo in the physical sample format.";
            }
            if (choice.physical_format().native_format() != card.native_format()){
                return "Choice " + std::to_string(i) + "'s physical format should be the 1ch x 96000 entry.";
            }
        }

        //  Preferred index: exact match wins; without a match the
        //  highest-scoring choice.
        if (list.preferred_index != 0){
            return "A 1ch x 96000 preferred format should match the native entry (index 0).";
        }
        //  No match: the L/R reinterpretation scores highest
        //  (0.5 x 2.0 x sqrt(48000) > 0.5 x 1.0 x sqrt(96000)), and the
        //  tie with R/L keeps the earlier index.
        AudioInputChoiceList bogus = input_choices(
            {card}, make_format(44100, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)
        );
        if (bogus.preferred_index != 1){
            return "A non-matching preferred format should fall back to the highest-scoring choice (L/R, index 1), got "
                + std::to_string(bogus.preferred_index) + ".";
        }
        AudioInputChoiceList logical_match = input_choices(
            {card}, make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)
        );
        if (logical_match.preferred_index != 1){
            return "A 2ch x 48000 stereo preferred format should match the L/R reinterpretation (index 1).";
        }

        //  No reinterpretations when other formats are supported...
        AudioInputChoiceList mixed = input_choices(
            {card, make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)}, card
        );
        if (mixed.choices.size() != 2
            || mixed.choices[0].is_reinterpreted() || mixed.choices[1].is_reinterpreted()){
            return "A device with other formats should not get reinterpretation entries.";
        }
        if (mixed.choices[0].display_name() != "2 x 48,000 Hz (Stereo)"
            || mixed.choices[1].display_name() != "1 x 96,000 Hz (Mono)"){
            return "Mixed device should list the native entries sorted by channel count desc, then sample rate desc.";
        }

        //  ...and the rule is specific to 1ch x 96000.
        if (input_choices(
            {make_format(44100, 1, QAudioFormat::ChannelConfig::ChannelConfigMono)},
            make_format(44100, 1, QAudioFormat::ChannelConfig::ChannelConfigMono)
        ).choices.size() != 1){
            return "A 1ch x 44100-only device should not get reinterpretation entries.";
        }
        if (input_choices(
            {make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)},
            make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo)
        ).choices.size() != 1){
            return "A 2ch x 48000-only device should not get reinterpretation entries.";
        }

        return true;
    }
};



class InputChoicesEmptyTest : public UnitTest{
public:
    InputChoicesEmptyTest()
        : UnitTest("AudioInfo - InputChoices_Empty")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const AudioFormat preferred = make_format(48000, 2);

        AudioInputChoiceList empty = input_choices({}, preferred);
        if (!empty.choices.empty() || empty.preferred_index != 0){
            return "An empty supported list should yield no choices and preferred_index 0.";
        }

        //  Invalid entries are dropped.
        QAudioFormat invalid;   //  zero rate / channels / unknown format
        AudioInputChoiceList invalid_only = input_choices({AudioFormat(invalid)}, preferred);
        if (!invalid_only.choices.empty()){
            return "Invalid supported entries should be dropped.";
        }

        return true;
    }
};



class InputChoiceEqualityTest : public UnitTest{
public:
    InputChoiceEqualityTest()
        : UnitTest("AudioInfo - InputChoice_Equality")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const AudioFormat stereo_48k = make_format(48000, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo);
        const AudioFormat stereo_441 = make_format(44100, 2, QAudioFormat::ChannelConfig::ChannelConfigStereo);
        const AudioFormat mono_96k = make_format(96000, 1, QAudioFormat::ChannelConfig::ChannelConfigMono);

        //  Same shape: equal. Different shape: not equal (symmetric).
        AudioInputChoice a(stereo_48k);
        AudioInputChoice b = AudioInputChoice(stereo_48k);
        AudioInputChoice c(stereo_441);
        if (!(a == b)){
            return "Native choices of the same shape should compare equal.";
        }
        if (a == c || c == a){
            return "Choices with different sample rates should not compare equal.";
        }

        //  Reinterpreted LR vs RL: the swap flag distinguishes them.
        const AudioInputChoice lr = AudioInputChoice::reinterpreted(AudioFormat(stereo_48k), AudioFormat(mono_96k), false);
        const AudioInputChoice rl = AudioInputChoice::reinterpreted(AudioFormat(stereo_48k), AudioFormat(mono_96k), true);
        if (lr == rl){
            return "Interleaved L/R and R/L choices should not compare equal.";
        }
        const AudioInputChoice lr_copy = AudioInputChoice::reinterpreted(AudioFormat(stereo_48k), AudioFormat(mono_96k), false);
        if (!(lr == lr_copy)){
            return "Identical reinterpreted choices should compare equal.";
        }
        //  A reinterpreted choice is not equal to the native one of the
        //  same logical shape.
        if (lr == a){
            return "A reinterpreted choice should not equal the native choice of the same shape.";
        }

        //  Invalid choices: equal to each other, not equal to any valid one.
        AudioInputChoice invalid;
        if (!(invalid == AudioInputChoice())){
            return "Two invalid choices should compare equal.";
        }
        if (invalid == a || a == invalid){
            return "An invalid choice should not equal a valid one.";
        }

        return true;
    }
};



}



void add_tests_AudioInfo(UnitTestDatabase& database){
    database.add<InputChoiceNamingTest>();
    database.add<InputChoiceFlagsTest>();
    database.add<InputChoicesSortDedupeTest>();
    database.add<InputChoicesCardQuirkTest>();
    database.add<InputChoicesEmptyTest>();
    database.add<InputChoiceEqualityTest>();
}



}
