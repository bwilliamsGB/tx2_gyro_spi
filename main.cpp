#include <chrono>
#include <iostream>
#include <thread>

#include "SparkFunLSM9DS1.h"
#
using namespace std;

int main(int argc, char** argv)
{
    cout << "Main, init gyro object" << endl;
    LSM9DS1 gyro;
    gyro.settings.device.commInterface = IMU_MODE_SPI;

    uint16_t retval = gyro.begin();
    cout << "Gyro begin return value: " << retval << endl;

    for(int i = 0; i < 30; i++) {
        // To read from the gyroscope, you must first call the
        // readGyro() function. When this exits, it'll update the
        // gx, gy, and gz variables with the most current data.
        gyro.readGyro();
        cout << "Read X: " << hex << gyro.gx << endl;
        cout << "Read Y: " << hex << gyro.gy << endl;
        cout << "Read Z: " << hex << gyro.gz << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


