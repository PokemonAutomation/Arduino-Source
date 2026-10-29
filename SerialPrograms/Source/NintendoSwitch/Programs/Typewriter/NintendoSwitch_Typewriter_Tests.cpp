/*  Typewriter Text Mappings - Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Pure data/logic tests for text_to_key_chords(): no hardware, no GUI.
 */

#include <cstdio>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch_Typewriter_Tests.h"
#include "NintendoSwitch_TypewriterTextMapping.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{

namespace{



std::string describe_text(const std::string& text){
    std::string out;
    for (unsigned char ch : text){
        switch (ch){
        case '\n':  out += "\\n";  break;
        case '\r':  out += "\\r";  break;
        case '\t':  out += "\\t";  break;
        default:
            if (ch < 0x20 || ch > 0x7e){
                char buffer[8];
                std::snprintf(buffer, sizeof(buffer), "\\x%02x", ch);
                out += buffer;
            }else{
                out += (char)ch;
            }
        }
    }
    return out;
}



class Test_TextToKeyChords : public UnitTest{
public:
    Test_TextToKeyChords(
        KeyboardLayout layout,
        std::string text,
        std::vector<std::vector<KeyboardKey>> expected,
        bool expect_throw
    )
        : UnitTest(
            std::string("NintendoSwitch::Typewriter::TextToKeyChords - ") +
            (layout == KeyboardLayout::QWERTY ? "QWERTY" : "AZERTY") +
            " \"" + describe_text(text) + "\"" +
            (expect_throw ? " (expect error)" : "")
          )
        , m_layout(layout)
        , m_text(std::move(text))
        , m_expected(std::move(expected))
        , m_expect_throw(expect_throw)
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        try{
            std::vector<std::vector<KeyboardKey>> chords =
                text_to_key_chords(m_layout, m_text);
            if (m_expect_throw){
                return UnitTestResult("Expected an error, but the text converted fine.");
            }
            if (chords != m_expected){
                return UnitTestResult("Key chords do not match the expected sequence.");
            }
            return UnitTestResult(true);
        }catch (OperationFailedException&){
            if (m_expect_throw){
                return UnitTestResult(true);
            }
            return UnitTestResult("Unexpected error.");
        }
    }

private:
    KeyboardLayout m_layout;
    std::string m_text;
    std::vector<std::vector<KeyboardKey>> m_expected;
    bool m_expect_throw;
};



//  UnitTestDatabase::add() forwards its arguments as a variadic template,
//  which cannot deduce a braced-init list; this helper takes the concrete
//  types so the call sites can stay readable.
void add_case(
    UnitTestDatabase& database,
    KeyboardLayout layout,
    std::string text,
    std::vector<std::vector<KeyboardKey>> expected,
    bool expect_throw
){
    database.add<Test_TextToKeyChords>(
        layout, std::move(text), std::move(expected), expect_throw
    );
}



}



void add_tests_TypewriterTextMapping(UnitTestDatabase& database){
    using namespace PokemonAutomation;

    //
    //  QWERTY: sample of the mapping, with the expected key and modifiers.
    //
    add_case(database, KeyboardLayout::QWERTY, "a", {{KeyboardKey::KEY_A}}, false);
    add_case(database, KeyboardLayout::QWERTY, "A", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_A}}, false);
    add_case(database, KeyboardLayout::QWERTY, "0", {{KeyboardKey::KEY_0}}, false);
    add_case(database, KeyboardLayout::QWERTY, " ", {{KeyboardKey::KEY_SPACE}}, false);
    add_case(database, KeyboardLayout::QWERTY, "!", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_1}}, false);
    add_case(database, KeyboardLayout::QWERTY, "#", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_3}}, false);
    add_case(database, KeyboardLayout::QWERTY, "_", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_MINUS}}, false);
    add_case(database, KeyboardLayout::QWERTY, "\"", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_APOSTROPHE}}, false);
    add_case(database, KeyboardLayout::QWERTY, "\t", {{KeyboardKey::KEY_TAB}}, false);
    add_case(database, KeyboardLayout::QWERTY, "\n", {{KeyboardKey::KEY_ENTER}}, false);
    add_case(
        database, KeyboardLayout::QWERTY, "Hello, World!",
        {
            {KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_H},
            {KeyboardKey::KEY_E},
            {KeyboardKey::KEY_L},
            {KeyboardKey::KEY_L},
            {KeyboardKey::KEY_O},
            {KeyboardKey::KEY_COMMA},
            {KeyboardKey::KEY_SPACE},
            {KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_W},
            {KeyboardKey::KEY_O},
            {KeyboardKey::KEY_R},
            {KeyboardKey::KEY_L},
            {KeyboardKey::KEY_D},
            {KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_1},
        },
        false
    );

    //
    //  Line endings: CRLF is a single Enter; a lone \r is an error.
    //
    add_case(database, KeyboardLayout::QWERTY, "\r\n", {{KeyboardKey::KEY_ENTER}}, false);
    add_case(
        database, KeyboardLayout::QWERTY, "abc\r\ndef",
        {
            {KeyboardKey::KEY_A},
            {KeyboardKey::KEY_B},
            {KeyboardKey::KEY_C},
            {KeyboardKey::KEY_ENTER},
            {KeyboardKey::KEY_D},
            {KeyboardKey::KEY_E},
            {KeyboardKey::KEY_F},
        },
        false
    );
    add_case(
        database, KeyboardLayout::QWERTY, "a\r\nb\r\nc",
        {
            {KeyboardKey::KEY_A},
            {KeyboardKey::KEY_ENTER},
            {KeyboardKey::KEY_B},
            {KeyboardKey::KEY_ENTER},
            {KeyboardKey::KEY_C},
        },
        false
    );
    add_case(database, KeyboardLayout::QWERTY, "\r", {}, true);
    add_case(database, KeyboardLayout::QWERTY, "abc\r", {}, true);
    add_case(database, KeyboardLayout::QWERTY, "a\rb", {}, true);

    //
    //  Unsupported characters.
    //
    add_case(database, KeyboardLayout::QWERTY, std::string(1, (uint8_t)0xe9), {}, true);  //  é
    add_case(database, KeyboardLayout::QWERTY, std::string(1, (uint8_t)0x01), {}, true);  //  control char
    add_case(database, KeyboardLayout::QWERTY, std::string(1, (uint8_t)0x7f), {}, true);  //  DEL

    //
    //  AZERTY: letters use the physical US position, digits need Shift,
    //  and the AltGr characters need the right Alt.
    //
    add_case(database, KeyboardLayout::AZERTY, "a", {{KeyboardKey::KEY_Q}}, false);
    add_case(database, KeyboardLayout::AZERTY, "A", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_Q}}, false);
    add_case(database, KeyboardLayout::AZERTY, "m", {{KeyboardKey::KEY_SEMICOLON}}, false);
    add_case(database, KeyboardLayout::AZERTY, "q", {{KeyboardKey::KEY_A}}, false);
    add_case(database, KeyboardLayout::AZERTY, "1", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_1}}, false);
    add_case(database, KeyboardLayout::AZERTY, "&", {{KeyboardKey::KEY_1}}, false);
    add_case(database, KeyboardLayout::AZERTY, "*", {{KeyboardKey::KEY_BACKSLASH}}, false);
    add_case(database, KeyboardLayout::AZERTY, "\t", {{KeyboardKey::KEY_TAB}}, false);
    add_case(database, KeyboardLayout::AZERTY, "\r\n", {{KeyboardKey::KEY_ENTER}}, false);
    add_case(database, KeyboardLayout::AZERTY, "#", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_3}}, false);
    add_case(database, KeyboardLayout::AZERTY, "@", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_0}}, false);
    add_case(database, KeyboardLayout::AZERTY, "[", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_5}}, false);
    add_case(database, KeyboardLayout::AZERTY, "]", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_MINUS}}, false);
    add_case(database, KeyboardLayout::AZERTY, "^", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_9}}, false);
    add_case(database, KeyboardLayout::AZERTY, "\\", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_8}}, false);
    add_case(database, KeyboardLayout::AZERTY, "|", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_6}}, false);
    add_case(database, KeyboardLayout::AZERTY, "~", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_2}}, false);
    add_case(database, KeyboardLayout::AZERTY, "\u0060", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_7}}, false);
    add_case(database, KeyboardLayout::AZERTY, "{", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_4}}, false);
    add_case(database, KeyboardLayout::AZERTY, "}", {{KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_EQUAL}}, false);
    add_case(database, KeyboardLayout::AZERTY, "<", {{KeyboardKey::KEY_102ND}}, false);
    add_case(database, KeyboardLayout::AZERTY, ">", {{KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_102ND}}, false);
    add_case(
        database, KeyboardLayout::AZERTY, "a <b> {c} d#e f@g h[i] j^k\\l|m ~n\u0060 o{p}q}r<s>t\n",
        {
            {KeyboardKey::KEY_Q},                                 //  a
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_102ND},                             //  <
            {KeyboardKey::KEY_B},                                 //  b
            {KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_102ND}, //  >
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_4},     //  {
            {KeyboardKey::KEY_C},                                 //  c
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_EQUAL}, //  }
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_D},                                 //  d
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_3},     //  #
            {KeyboardKey::KEY_E},                                 //  e
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_F},                                 //  f
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_0},     //  @
            {KeyboardKey::KEY_G},                                 //  g
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_H},                                 //  h
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_5},     //  [
            {KeyboardKey::KEY_I},                                 //  i
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_MINUS}, //  ]
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_J},                                 //  j
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_9},     //  ^
            {KeyboardKey::KEY_K},                                 //  k
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_8},     //  backslash
            {KeyboardKey::KEY_L},                                 //  l
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_6},     //  |
            {KeyboardKey::KEY_SEMICOLON},                         //  m
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_2},     //  ~
            {KeyboardKey::KEY_N},                                 //  n
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_7},     //  `
            {KeyboardKey::KEY_SPACE},                             //   
            {KeyboardKey::KEY_O},                                 //  o
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_4},     //  {
            {KeyboardKey::KEY_P},                                 //  p
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_EQUAL}, //  }
            {KeyboardKey::KEY_A},                                 //  q
            {KeyboardKey::KEY_RIGHT_ALT, KeyboardKey::KEY_EQUAL}, //  }
            {KeyboardKey::KEY_R},                                 //  r
            {KeyboardKey::KEY_102ND},                             //  <
            {KeyboardKey::KEY_S},                                 //  s
            {KeyboardKey::KEY_LEFT_SHIFT, KeyboardKey::KEY_102ND}, //  >
            {KeyboardKey::KEY_T},                                 //  t
            {KeyboardKey::KEY_ENTER},                             //  \n
        },
        false
    );
    add_case(database, KeyboardLayout::AZERTY, std::string(1, (uint8_t)0xe9), {}, true);  //  é
}




}
}
}
