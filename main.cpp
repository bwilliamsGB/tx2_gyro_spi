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

    for(int i = 0; i < 100; i++) {
        // To read from the gyroscope, you must first call the
        // readGyro() function. When this exits, it'll update the
        // gx, gy, and gz variables with the most current data.
        gyro.readGyro();
        cout << "Calculated Gyro xyz: " << hex << gyro.calcMag(gyro.gx) <<
                " " << hex << gyro.calcMag(gyro.gy) <<
                " " << hex << gyro.calcMag(gyro.gz) << endl;
        cout << "Raw Gyro xyz: " << hex << gyro.gx <<
                " " << hex << gyro.gy <<
                " " << hex << gyro.gz << endl;

        gyro.readMag();
        cout << "Calculated Mag x: " << hex << gyro.calcMag(gyro.mx) <<
                " " << hex << gyro.calcMag(gyro.my) <<
                " " << hex << gyro.calcMag(gyro.mz) << endl;
        cout << "Raw Mag x: " << hex << gyro.mx <<
                " " << hex << gyro.my <<
                " " << hex << gyro.mz << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


