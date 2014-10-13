#include "power_monitor.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

int main()
{
	
	float voltage;
        double current;
        double power;
        double maxpower;
        int i, j,k;
        int count = 5;
        int iic_fd;

   //EXPERIMENT LOG FILE---------------------------------------
   char log_filename[80];
   sprintf(log_filename, "pow_res.csv");
   ofstream log_file;
   log_file.open(log_filename, std::ofstream::out | std::ofstream::app);
   //----------------------------------------------------------

        double totalPower = 0;

        iic_fd = open("/dev/i2c-9", O_RDWR);
        if (iic_fd < 0) {
                printf("ERROR: Unable to open /dev/i2c-9 for PMBus access: %d\n", iic_fd);
                exit(1);
        }

        j = sizeof(zc702_rails) / sizeof(struct voltage_rail);

while(1)
{
	//cout << clock();
	log_file << clock();		

        totalPower = 0.0f;
        power = 0.0f;
        for(i = 0; i < j; i++) {
                zc702_rails[i].average_power = 0;
                zc702_rails[i].average_current=0;
        }


	for(k = 0; k < count; k++) {
        for(i = 0; i < j; i++) {
                voltage = readVoltage(iic_fd, zc702_rails[i].device, zc702_rails[i].page);
                current = readCurrent(iic_fd, zc702_rails[i].device, zc702_rails[i].page);
                power = voltage * current * 1000/count;
                //totalPower += power;
                zc702_rails[i].average_current += current/count;
                zc702_rails[i].average_power += power;
                if(k == count-1) {
          //              cout << ",  " << zc702_rails[i].average_power;
                        log_file << ", " << zc702_rails[i].average_power;
                        totalPower = totalPower + zc702_rails[i].average_power;
                }
        }
        }
	//cout << endl;
	log_file << endl;
	
}
}
