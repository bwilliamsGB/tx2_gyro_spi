#include <SPI.h>

void setup (void)
{
    Serial.begin(9600);

    // disable Slave Select
    digitalWrite(SS, HIGH);

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    SPI.setDataMode(SPI_MODE0);
}


void loop (void)
{
    char c;
    char reply;

    // enable Slave Select
    digitalWrite(SS, LOW);

    Serial.println("Send Hello World");
    Serial.print("Reply: ");

    // send test string
    for (const char * p = "Hello, world!\r" ; c = *p; p++) {
        reply = SPI.transfer(c);
        Serial.print(reply);
    }
    Serial.println("");

    digitalWrite(SS, HIGH); // disable Slave Select
    delay(2000);
}
