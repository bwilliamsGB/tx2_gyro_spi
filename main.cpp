#include <chrono>
#include <iostream>
#include <thread>

extern "C" {
  #include "spidev_test.h"
  #include "jetsonGPIO.h"
}

using namespace std;

#define VERSION_NUM 1

// chip select to SCC2230 pin 13 (CSB) on MuRata header
#define cs_Z_2230 9

// chip select to SCC2130 pin 9 (EXTRA1) on MuRata header
#define cs_Y_2130 8

// chip select to SCC2100  pin 8 (EXTRA2) on MuRata header
#define cs_X_2100 7

#define NUM_AXIS 3

// hard external reset to all 3 devices.
#define nRESET 6

#define MOSI 11
#define MISO 12
#define SCLK 13
#define SPI_CLK_RATE  4000000
#define MODE_ALL_CLEAR_0  0b00100101  //0x08 no shifting, sent as is
#define SPI_BITS_PER_WORD  8  // number of bits (Only 8 works)
#define BAUD_RATE 9600

// murata sensor SPI commands
// Standard Requests
const unsigned long REQ_READ_RATE    = 0x040000f7;
const unsigned long REQ_READ_ACC_X   = 0x100000e9;
const unsigned long REQ_READ_ACC_Y   = 0x140000ef;
const unsigned long REQ_READ_ACC_Z   = 0x180000e5;
const unsigned long REQ_READ_TEMP    = 0x1c0000e3;
const unsigned long REQ_WRITE_FLT_60 = 0xfc200006;
const unsigned long REQ_WRITE_FLT_10 = 0xfc1000c7;
const unsigned long REQ_READ_STAT_SUM   = 0x7c0000b3;
const unsigned long REQ_READ_RATE_STAT1 = 0x240000c7;
const unsigned long REQ_READ_RATE_STAT2 = 0x280000cd;
const unsigned long REQ_READ_ACC_STAT   = 0x3c00003d;
const unsigned long REQ_READ_COM_STAT1  = 0x6c0000ab;

// Special requests
const unsigned long REQ_HARD_RESET = 0xD8000431;

// Function prototypes
unsigned long initialise_device(int);
unsigned long spi_transfer(unsigned long, int);
void reset_sensor_module(void);
int  check_status_ok(unsigned long);
void test_status(void);
uint8_t calc_crc(unsigned long);
static uint8_t CRC8(unsigned long, uint8_t);

void loop()
{
    unsigned long response_stat_sum;
    unsigned long rate;
    unsigned long acc_x;
    unsigned long acc_y;
    unsigned long acc_z;
    const char *ident[NUM_AXIS];
    int chip_select_list[NUM_AXIS],chip_select, i;
    char srate[12], sacc_x[12], sacc_y[12], sacc_z[12];

    chip_select_list[0] = cs_Z_2230;
    chip_select_list[1] = cs_Y_2130;
    chip_select_list[2] = cs_X_2100;
    ident[0] = "cs_Z_2230";
    ident[1] = "cs_Y_2130";
    ident[2] = "cs_X_2100";

    for (int s=0; s < 100; s++) {
        // All read commands are 1 frame delayed so
        // when a write is performed, the read data is
        // from a register of the previous write.
        for (chip_select = 0; chip_select < NUM_AXIS; chip_select++) {
            response_stat_sum = spi_transfer(REQ_READ_RATE, chip_select);
        }

        //Serial.println("ID, rate, a_x, a_y, a_z");
        cout << "ID, rate, a_x, a_y, a_z" << endl;

        for(i = 0; i < NUM_AXIS; i++) {
            chip_select = i;
            rate =  spi_transfer(REQ_READ_ACC_X, chip_select_list[chip_select]);
            acc_x = spi_transfer(REQ_READ_ACC_Y, chip_select_list[chip_select]);
            acc_y = spi_transfer(REQ_READ_ACC_Z, chip_select_list[chip_select]);    
            acc_z = spi_transfer(REQ_READ_RATE, chip_select_list[chip_select]);

            // debug section //
            sprintf(srate, "%lu", ( rate >> 8UL) & 0xFFFF);
            sprintf(sacc_x, "%lu", (acc_x >> 8UL) & 0xFFFF);
            sprintf(sacc_y, "%lu", (acc_y >> 8UL) & 0xFFFF);
            sprintf(sacc_z, "%lu", (acc_z >> 8UL) & 0xFFFF);

            cout << ident[chip_select] << ", " <<
                    srate  << ", " <<
                    sacc_x << ", " <<
                    sacc_y << ", " << sacc_z << endl;

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

//***************************************//
// Functions
//***************************************//
void setup(void)
{
    unsigned long response_stat_sum;
    const char *ident[NUM_AXIS];
    int chip_select_list[NUM_AXIS], chip_select, i;

    cout << "Murata Orientation Sensor Monitor V" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    reset_sensor_module();

    // Initialise cs_Z_2230 (SCC2230)
    response_stat_sum = initialise_device(cs_Z_2230);
    if (check_status_ok(response_stat_sum) == 0) {
        cout << "SCC2230 has failed Initialisation" << endl;
    }

    // only enable 1 device for now, add these back in for using
    // all three axis
    //response_stat_sum = initialise_device(cs_Y_2130);  
    //if (check_status_ok(response_stat_sum) == 0) {
    //    cout << "SCC2130 has failed Initialisation" << endl;
    //}

    //response_stat_sum = initialise_device(cs_X_2100);  
    //if (check_status_ok(response_stat_sum) == 0) {
    //    cout << "SCC2100 has failed Initialisation" << endl;
    //}

    //test_status();
      
}

//***************************************//
void reset_sensor_module()
{
    int reset = 398;

    gpioUnexport(reset);
    gpioExport(reset);
    gpioSetDirection(reset, outputPin);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    //digitalWrite(nRESET, LOW);
    gpioSetValue(reset, off);

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    //digitalWrite(nRESET, HIGH);
    gpioSetValue(reset, on);

    // wait 25ms for reset to complete and SPI to become active
    std::this_thread::sleep_for(std::chrono::milliseconds(35));
}

//***************************************//
unsigned long initialise_device(int ncs)
{
    unsigned long response_stat_sum;
     init_spi();
 
 
    spi_transfer(REQ_WRITE_FLT_60, ncs);
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    // Clear Status Registers
    spi_transfer(REQ_READ_RATE_STAT1,ncs);
    spi_transfer(REQ_READ_RATE_STAT2,ncs);
    spi_transfer(REQ_READ_ACC_STAT,ncs);
    spi_transfer(REQ_READ_COM_STAT1,ncs);
    spi_transfer(REQ_READ_STAT_SUM,ncs);

    // read a 2nd time because of out of frame format
    response_stat_sum = spi_transfer(REQ_READ_STAT_SUM,ncs);

    // read a 3rd time to be safe (maybe unnecessary)
    response_stat_sum = spi_transfer(REQ_READ_STAT_SUM,ncs);

    cout << "ResponseStatSum = " << hex << response_stat_sum << endl;
    return response_stat_sum;
  }


//***************************************//
unsigned long spi_transfer(unsigned long data, int ncs)
{
/*  Original send, ported to TX2 version below
    unsigned long buffer;

    // assert chip select
    digitalWrite(ncs, LOW);

    // send ms byte of data and read back and place in msbyte of buffer
    buffer = (unsigned long)SPI.transfer((data>>24)) << 24UL; 
    buffer |= (unsigned long)SPI.transfer((data>>16)) << 16UL;
    buffer |= (unsigned long)SPI.transfer((data>>8)) << 8UL;
    buffer |= (unsigned long)SPI.transfer(data);

    // deassert chip select
    digitalWrite(ncs, HIGH);

    return buffer;
*/

    unsigned long buffer;
    uint8_t rx_byte = 0;
    uint8_t tx_byte = 0;

    tx_byte = (data>>24);
    old_transfer(&tx_byte, &rx_byte, 1);
    buffer = (unsigned long)(rx_byte << 24UL);

    tx_byte = (data>>16);
    old_transfer(&tx_byte, &rx_byte, 1);
    buffer |= (unsigned long)(rx_byte << 16UL);

    tx_byte = (data>>8);
    old_transfer(&tx_byte, &rx_byte, 1);
    buffer |= (unsigned long)(rx_byte << 8UL);

    tx_byte = data;
    old_transfer(&tx_byte, &rx_byte, 1);
    buffer |= rx_byte;
}

//***************************************//
int check_status_ok(unsigned long resp_stat_sum)
{
    // check that RS bits are set to 0x01 for normal operation
    if (((resp_stat_sum >> 24) & 0x03) != 0x01) {
        return 0;
    }
    else {
        return 1;
   }
}

//***************************************//
int calculate_temperature (unsigned long temp)
{
    int temperature;

    // need to check if the casting is correct
    temperature = (int)temp;
    temperature = 60 + (temperature / 14.7);
    return temperature;
}


//***************************************//
void test_status()
{
    char gyro_status, acc_status;
    unsigned long temp;
    unsigned long response_stat_sum;
    int temperature;

    spi_transfer(REQ_READ_STAT_SUM,cs_Z_2230);

    // perfrom this twice to read the out of frame status summary
    response_stat_sum = spi_transfer(REQ_READ_STAT_SUM,cs_Z_2230);

    gyro_status = (response_stat_sum >> 8) & 0x01;
    acc_status = (response_stat_sum >> 11) & 0x01;
    temp = spi_transfer(REQ_READ_TEMP, cs_Z_2230);
    temp = (spi_transfer(REQ_READ_TEMP, cs_Z_2230) >> 8UL) & 0xFFFF;

    temperature = calculate_temperature(temp);
    cout << "response_stat_sum =0x" << hex << response_stat_sum;

    cout << "gyro status = " << gyro_status << endl;
    cout << "accel status = " << acc_status << endl;
    cout << "Temperature = " << temperature << endl;

    spi_transfer(REQ_READ_RATE_STAT1, cs_Z_2230);
    response_stat_sum = spi_transfer(REQ_READ_RATE_STAT1, cs_Z_2230);
    cout << "RATE Status 1 Register =0x" << hex << response_stat_sum << endl;

    spi_transfer(REQ_READ_RATE_STAT2, cs_Z_2230);
    response_stat_sum = spi_transfer(REQ_READ_RATE_STAT2, cs_Z_2230);
    cout << "RATE Status 2 Register = 0x" << hex << response_stat_sum << endl;

    spi_transfer(REQ_READ_COM_STAT1, cs_Z_2230);
    response_stat_sum = spi_transfer(REQ_READ_COM_STAT1, cs_Z_2230);
    cout << "COMMON Status Register = 0x" << hex << response_stat_sum << endl;

    spi_transfer(REQ_READ_ACC_STAT, cs_Z_2230);
    response_stat_sum = spi_transfer(REQ_READ_ACC_STAT, cs_Z_2230);
    cout << "ACC Status Register = 0x" << hex << response_stat_sum << endl;
}


//***************************************//
// Calculate CRC for 24 MSB's of the 32 bit dword
// (8 LSB's are the CRC field and are not included in CRC calculation)
uint8_t calc_crc(unsigned long data)
{
    unsigned long BitMask;
    unsigned long BitValue;
    uint8_t CRC = 0xFF;
    for (BitMask = 0x80000000; BitMask != 0x80; BitMask >>= 1)
    {
        BitValue = data & BitMask;
        CRC = CRC8(BitValue, CRC);
    }
    CRC = (uint8_t)~CRC;
    return CRC;
}


//***************************************//
static uint8_t CRC8(unsigned long BitValue, uint8_t CRC)
{
    uint8_t Temp = (uint8_t)(CRC & 0x80);
    if (BitValue != 0)
    {
        Temp ^= 0x80;
    }
    CRC <<= 1;
    if (Temp > 0)
    {
        CRC ^= 0x1D;
    }
    return CRC;
}


int main(int argc, char** argv)
{
    cout << "Main, init gyro object" << endl;
    setup();
    loop();
    close_spi();
}



