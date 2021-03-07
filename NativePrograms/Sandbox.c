





////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
//#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
//#include "DeviceFramework/Controller.h"
//#include "PokemonSwShLibraries/PokemonRoutines.h"
//#include "PokemonSwShLibraries/CodeEntry.h"
//#include "PokemonSwShLibraries/ProgramFlow.h"
//#include "PokemonSwShLibraries/NavigateDateTime.h"
#include "DeviceFramework/HardwareLED.h"
//#include "Libraries/AutoHostTools.h"
//#include "Libraries/DaySkipperTools.h"
//#include "Libraries/EggHelpers.h"
//#include "PokemonSwShLibraries/ShinyHuntTools.h"
//#include "Framework/BotBaseUtils.h"
//#include "Framework/ProtocolSerial.h"
//#include "Framework/SerialCommandsPBF.h"
//#include "Framework/SerialCommandsSSF.h"
//#include "Framework/SerialProcessor.h"
#include "NativePrograms/SwitchFramework/uart.h"
#include "NativePrograms/SwitchFramework/HardwareSerial.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"


#if 0
#define BAUD_RATE 9600
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
// write a string to the uart
#define uart_print(s) uart_print_P(PSTR(s))
void uart_print_P(const char *str)
{
    char c;
    while (1) {
        c = pgm_read_byte(str++);
        if (!c) break;
        uart_putchar(c);
    }
}

void blink(uint8_t x){
    while (x--){
        onboard_led(true);
        pbf_wait(50);
        onboard_led(false);
        pbf_wait(50);
    }
}
#endif

int main(void){
    setup_leds();
    initialize_framework(PABB_PID_SANDBOX);
    bool state = true;

    while (true){
        onboard_led(state = !state);
        uart_putchar('h');
//        pabb_send_error_ready();
//        pbf_wait(5 * TICKS_PER_SECOND);
//        pbf_press_button(BUTTON_ZL, 10, 5 * TICKS_PER_SECOND - 10);
    }


#if 0
    setup_leds();
    CPU_PRESCALE(0);  // run at 16 MHz

    Serial_Init(BAUD_RATE, false);

    bool state = true;
    onboard_led(state);

    while (true){
        if (Serial_IsCharReceived()){
            onboard_led(state = !state);
            char ch = (char)Serial_ReceiveByte();
            Serial_SendByte();
        }
//        Serial_SendByte('H');
//        Serial_SendByte('e');
//        onboard_led(state = !state);
//        uart_print("H");
//        uart_print("Hello World\r\n");
    }
#endif



#if 0
    setup_leds();
    initialize_framework();

    CPU_PRESCALE(0);  // run at 16 MHz
    uart_init(BAUD_RATE);

//    bool state = false;
    onboard_led(false);

    while (true){
        if (uart_available()){
//            onboard_led(state = !state);
            char ch = uart_getchar();
            onboard_led(true);
            pbf_wait(50);
            onboard_led(false);
            pbf_wait(125);

            switch (ch){
            case 'a': blink(2); break;
            case 's': blink(3); break;
            }
            uart_putchar(ch);
        }
//        onboard_led(state = !state);
//        uart_print("H");
//        uart_print("Hello World\r\n");
    }
#endif


#if 0
    setup_leds();

    CPU_PRESCALE(0);  // run at 16 MHz
    uart_init(BAUD_RATE);

    bool state = true;
    onboard_led(state);

    uint8_t c;
    uart_print("UART Example\r\n");
    while (1) {
        if (uart_available()) {
            onboard_led(state = !state);
            c = uart_getchar();
            uart_print("Byte: ");
            uart_putchar(c);
            uart_putchar('\r');
            uart_putchar('\n');
        }
    }
#endif



#if 0
    start_program_callback();

    initialize_framework(true);

//    Serial_Init(9600, false);
//    sei();
    CPU_PRESCALE(0);  // run at 16 MHz
    uart_init(9600);

    start_program_flash(CONNECT_CONTROLLER_DELAY);
    grip_menu_connect_go_home();

    bool leds = false;
    while (true){
        onboard_led(leds);
        leds = !leds;

        uart_putchar('H');
        uart_putchar('e');
//        uart_putchar('l');
//        uart_putchar('l');
//        uart_putchar('o');
//        uart_putchar('\r');
//        uart_putchar('\n');

        pbf_wait(TICKS_PER_SECOND);
    }
#endif








}

