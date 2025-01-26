/*  Digit Entry
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

//#include <cstring>
//#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Commands_DigitEntry.h"
//#include "NintendoSwitch_Messages_DigitEntry.h"


#if 0
void enter_digits_str(uint8_t count, const char* digits){
    enter_digits(count, (const uint8_t*)digits);
}
void enter_digits(uint8_t count, const uint8_t* digits){
    enter_digits(*PokemonAutomation::global_connection, count, digits);
}
#endif


namespace PokemonAutomation{
namespace NintendoSwitch{


#define CODE_ENTRY_FAST

//  3 seems to work maybe 95% of the time.
#define CODE_DELAY      4



const uint8_t XCORD[] = {1, 0, 1, 2, 0, 1, 2, 0, 1, 2};
const uint8_t YCORD[] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2};

void code_entry_scroll(SwitchControllerContext& context, DpadPosition direction){
    pbf_wait(context, CODE_DELAY);
    ssf_issue_scroll(context, direction, 0);
}
uint16_t scroll_to(
    SwitchControllerContext& context,
    uint8_t start_digit, uint8_t end_digit, bool actually_scroll
){
    //  Returns the # of ticks needed to scroll from "start_digit" to "end_digit".
    //  If "actually_scroll" is true, then it does the scrolling as well.

    uint8_t xcoord = XCORD[start_digit];
    uint8_t ycoord = YCORD[start_digit];

    uint16_t cost = 0;

    //  These shortcuts over the OK and backspace buttons aren't stable.
//#ifdef CODE_ENTRY_FAST
//    //  Special cases
//    if (end_digit == 1 && (start_digit == 0 || start_digit == 9)){
//        if (actually_scroll){
//            code_entry_scroll(SSF_SCROLL_RIGHT);
//            code_entry_scroll(SSF_SCROLL_UP);
//            pbf_wait(1);
//            code_entry_scroll(SSF_SCROLL_RIGHT);
//        }
//        cost += 3 * CODE_DELAY;
//        xcoord = 0;
//        ycoord = 0;
//    }
//    if (end_digit == 3 && (start_digit == 0 || start_digit == 7)){
//        if (actually_scroll){
////            pbf_wait(context, 1);
//            code_entry_scroll(SSF_SCROLL_LEFT);
////            pbf_wait(context, 1);
//            code_entry_scroll(SSF_SCROLL_UP);
////            pbf_wait(context, 1);
//            code_entry_scroll(SSF_SCROLL_LEFT);
//        }
//        cost += 3 * CODE_DELAY;
//        xcoord = 2;
//        ycoord = 0;
//    }
//#endif

    //  Use nearest path algorithm.
    uint8_t desired_x = XCORD[end_digit];
    uint8_t desired_y = YCORD[end_digit];
    while (xcoord != desired_x || ycoord != desired_y){
        //  If we need to move up, do it first since it's always safe to do so.
        if (ycoord > desired_y){
            if (actually_scroll){
                code_entry_scroll(context, SSF_SCROLL_UP);
            }
            cost += CODE_DELAY;
            ycoord--;
            continue;
        }

        //  Now we move down to the correct Y-coordinate.
        //  If the digit is zero, we can go straight down to the bottom and
        //  it will automatically move to zero even if the X-coordinate is wrong.
        if (ycoord < desired_y){
            if (actually_scroll){
                code_entry_scroll(context, SSF_SCROLL_DOWN);
            }
            cost += CODE_DELAY;
            ycoord++;

            //  If we land on zero, the X-axis is set to 1.
            if (ycoord == 3){
                xcoord = 1;
            }
            continue;
        }

        //  Now it's safe to adjust the X-coordinate since we will have
        //  moved away from the bottom row if the digit is not zero.
        if (xcoord < desired_x){
            if (actually_scroll){
                code_entry_scroll(context, SSF_SCROLL_RIGHT);
            }
            cost += CODE_DELAY;
            xcoord++;
            continue;
        }
        if (xcoord > desired_x){
            if (actually_scroll){
                code_entry_scroll(context, SSF_SCROLL_LEFT);
            }
            cost += CODE_DELAY;
            xcoord--;
            continue;
        }
    }

    return cost;
}







void enter_digits(SwitchControllerContext& context, uint8_t count, const uint8_t* digits){
#if 0
    context.issue_request(DeviceRequest_enter_digits(count, digits));
#else
    //  Enter code.
    uint8_t last_digit = 1;
    uint8_t s = 0;
    uint8_t e = count;

#ifdef CODE_ENTRY_FAST
    bool shift_left = false;
    while (s < e){
        //  Calculate the distance to both the first and last digit.
        //  Pick the closer one.
        uint8_t front = digits[s];
        uint8_t back  = digits[e - 1];
        if (front >= '0'){
            front -= '0';
        }
        if (back >= '0'){
            back -= '0';
        }
        if (front > 9){
            front = 0;
        }
        if (back > 9){
            back = 0;
        }

        //  Calculate costs to scroll to destination.
        uint16_t cost0 = scroll_to(context, last_digit, front, false);
        uint16_t cost1 = scroll_to(context, last_digit, back, false);

        //  Now actually scroll to the destination.
        uint8_t digit = cost0 <= cost1 ? front : back;
        cost1 = scroll_to(context, last_digit, digit, true);

        //  If no scrolling is needed, we still need to wait a non-zero time.
        //
        //  This wait doesn't actually slow anything down since the A button
        //  needs to wait 8 cycles anyway. Instead, this wait delays the
        //  LB button so that it doesn't happen the same time as the A button
        //  from the previous iteration.
        if (cost1 == 0){
            pbf_wait(context, CODE_DELAY);
        }
        if (shift_left){
            ssf_press_button(context, BUTTON_L, 0);
        }

        //  Enter digit.
        ssf_press_button(context, BUTTON_A, 0);

        //  Update pointers.
        if (cost0 <= cost1){
            shift_left = false;
            s++;
        }else{
            shift_left = true;
            e--;
        }
        last_digit = digit;
    }
#else
    while (s < e){
        uint8_t digit = digits[s];
        if (digit > 9){
            digit = 0;
        }

        //  Scroll to digit.
        scroll_to(context, last_digit, digit, true);

        //  Enter digit.
        ssf_press_button(context, BUTTON_A, 0);

        s++;
        last_digit = digit;
    }
#endif
    pbf_wait(context, CODE_DELAY);
    ssf_press_button(context, BUTTON_PLUS, CODE_DELAY);
    ssf_press_button(context, BUTTON_PLUS, CODE_DELAY);
#endif
}


void enter_digits_str(SwitchControllerContext& context, uint8_t count, const char* digits){
    enter_digits(context, count, (const uint8_t*)digits);
}
uint8_t convert_digit(uint8_t digit){
    if (digit >= '0'){
        digit -= '0';
    }
    if (digit > 9){
        digit = 0;
    }
    return digit;
}

#if 0
int register_message_converters_switch_digit_entry(){
    register_message_converter(
        PABB_MSG_COMMAND_ENTER_DIGITS,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "enter_digits() - ";
            if (body.size() != sizeof(pabb_enter_digits)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_enter_digits*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", count = " << (unsigned)params->count;
            ss << ", digits = ";
            for (uint8_t c = 0; c < params->count; c++){
                ss << ((params->digit_pairs[c / 2] >> 4 * (c & 1)) & 0x0f);
            }
            return ss.str();
        }
    );
    return 0;
}
int init_SwitchDigitEntry = register_message_converters_switch_digit_entry();
#endif


}
}
