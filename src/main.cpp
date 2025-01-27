#include <Arduino.h>

// select only NEC and the universal decoder for pulse distance protocols
// #define DECODE_NEC            // Includes Apple and Onkyo
// #define DECODE_DISTANCE_WIDTH // In case NEC is not received correctly. Universal decoder for pulse distance width protocols

// #define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
// #define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active
// #define NO_LED_FEEDBACK_CODE      // saves 92 bytes program memory
#define RECORD_GAP_MICROS 12000   // Default is 8000. Activate it for some LG air conditioner protocols
#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
// #define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

// #define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#define DELAY_AFTER_SEND 2000
#define DELAY_AFTER_LOOP 5000
#define IR_RECEIVE_PIN 26
#define IR_SEND_PIN 25

void setup()
{
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " + IR_RECEIVE_PIN));
    IrSender.begin(IR_SEND_PIN); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
    Serial.println(F("Send IR signals at pin " + IR_SEND_PIN));
}

uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;
uint8_t sRepeats = 1;

/*
 * Send NEC IR protocol
 */
void send_ir_data()
{
    Serial.print(F("Sending: 0x"));
    Serial.print(sAddress, HEX);
    Serial.print(sCommand, HEX);
    Serial.println(sRepeats, HEX);
    Serial.flush(); // To avoid disturbing the software PWM generation by serial output interrupts

    // clip repeats at 4
    if (sRepeats > 4)
    {
        sRepeats = 4;
    }
    // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
    IrSender.sendNEC(sAddress, sCommand, sRepeats);
}

void receive_ir_data()
{
    if (IrReceiver.decode())
    {
        Serial.print(F("Decoded protocol: "));
        Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));
        Serial.print(F(", decoded raw data: "));
        PrintULL::print(&Serial, IrReceiver.decodedIRData.decodedRawData, HEX);
        Serial.print(F(", decoded address: "));
        Serial.print(IrReceiver.decodedIRData.address, HEX);
        Serial.print(F(", decoded command: "));
        Serial.println(IrReceiver.decodedIRData.command, HEX);
        IrReceiver.resume();
    }
}

void loop()
{
    /*
     * Print loop values
     */
    Serial.println();
    Serial.print(F("address=0x"));
    Serial.print(sAddress, HEX);
    Serial.print(F(" command=0x"));
    Serial.print(sCommand, HEX);
    Serial.print(F(" repeats="));
    Serial.println(sRepeats);
    Serial.flush();

    send_ir_data();
    IrReceiver.restartAfterSend(); // Is a NOP if sending does not require a timer.

    // wait for the receiver state machine to detect the end of a protocol
    delay((RECORD_GAP_MICROS / 1000) + 5);
    receive_ir_data();

    // Prepare data for next loop
    sAddress += 0x0101;
    sCommand += 0x11;
    sRepeats++;

    delay(3000); // Loop delay
}
