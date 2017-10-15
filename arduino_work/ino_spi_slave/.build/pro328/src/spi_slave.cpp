#include <Arduino.h>
#include <SPI.h>
void setup (void);
void loop (void);
#line 1 "src/spi_slave.ino"
//#include <SPI.h>
char buff [50];
char reply_buff[] = "message reply message reply message reply\r\n";
volatile byte index;
volatile boolean process;
volatile boolean got_interrupt;
volatile byte answer = 0xaa;

void setup (void)
{
    Serial.begin(9600);
    pinMode(MISO, OUTPUT);

    // turn on SPI in slave mode
    SPCR |= _BV(SPE);

    // turn on interrupts
    SPCR |= _BV(SPIE);

    SPDR = 0; // initialize SPI register

    index = 0;
    process = false;
    got_interrupt = false;
    SPI.attachInterrupt();
    //SPI.setClockDivider(SPI_CLOCK_DIV8);
    Serial.println("Slave initialized");
}


// SPI interrupt routine
ISR (SPI_STC_vect)
{
    got_interrupt = true;
    // read byte from SPI Data Register
    byte c = SPDR; 
    if (index < sizeof(buff)) {
        // save data in the next index in the array buff
        buff[index++] = c;

        // Setup the reply
        //SPDR = reply_buff[index];
        SPDR = answer++;

        // check for the end of the word
        //if (c == '\r' && index >= 10) {
        //if (c == '\r' || index > sizeof(buff)) {
            process = true;
            index = 0;
        //}
   }
}


void loop (void) {
    if (process) {
        process = false;

        Serial.print("Received: 0x");
        for (int i = 0; i < 1; i++) {
            Serial.print((int)buff[i], HEX);
            Serial.print(", 0x");
        }
        Serial.println("");

        // print the array on serial monitor
        //Serial.print("Full Message received: ");
        //Serial.println(buff);

        // reset buffer
        memset(buff, 0, sizeof(buff));
        index = 0;
    }
}
