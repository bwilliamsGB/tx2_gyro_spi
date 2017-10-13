#include <SPI.h>
char buff [50];
char reply_buff[] = "message reply message reply message reply\r\n";
volatile byte index;
volatile boolean process;
volatile boolean got_interrupt;

void setup (void)
{
    Serial.begin(9600);
    pinMode(MISO, OUTPUT);

    // turn on SPI in slave mode
    SPCR |= _BV(SPE);

    // turn on interrupts
    SPCR |= _BV(SPIE);

    SPDR = 0; // initialise SPI register

    index = 0;
    process = false;
    got_interrupt = false;
    SPI.attachInterrupt();
    SPI.setClockDivider(SPI_CLOCK_DIV8);
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
        buff [index++] = c;

        // Setup the reply
        SPDR = reply_buff[index];

        // check for the end of the word
        //if (c == '\r' && index >= 10) {
        if (c == '\r' || index > sizeof(buff)) {
            process = true;
            index = 0;
        }
   }
}


void loop (void) {
    if (process) {
        process = false;

        // print the array on serial monitor
        Serial.print("Full Message received: ");
        Serial.println(buff);

        // reset button to zero
        index = 0;
    }
}
