#include "stdlib.h"
#include "time.h"
#include "stdio.h"
#include "unistd.h"
#include <pthread.h>
#include <iostream>
#include <fstream>

#include "power_monitor.h"
#include "sw_freq_changer.h"

#define MAX_SCRUBS 5

using namespace std;

int done;

//function that is run by the scrubbing thread
void *scrubber(void * fdiv)
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
                return 0;
        }

        j = sizeof(zc702_rails) / sizeof(struct voltage_rail);


	while(done==0)
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
          //                      cout << ",  " << zc702_rails[i].average_power;
                                log_file << ", " << zc702_rails[i].average_power;
                                totalPower = totalPower + zc702_rails[i].average_power;
                        }
                }
                }
                //cout << endl;
                log_file << endl;

        }

	return NULL;
}

int main(int argc, char *argv[])
{	
	if(argc != 2)
        {
                printf("Invalid input arguments expected: %s fdiv\n", argv[0]);
                return -1;
        }

	int fdiv = atoi(argv[1]); //get the scrubbing period from the input argument
	
	arm_pll_data freq_scaler;
	setup_arm_pll(&freq_scaler);

	done = 0; //global variable to indicate that the scrubbing has finished (quick and dirty hack)

	struct timespec tim, tim2;

	pthread_t scrub_thread;

	set_fdiv_value(&freq_scaler, fdiv);
	//Start the scrubbing thread and then we can start the energy measurement...
	if(pthread_create(&scrub_thread, NULL, scrubber, &fdiv)) 
	{
		fprintf(stderr, "Error creating the scrubber thread.\n");
		return 1;
	}

	int i=0;
	for (i=0;i<MAX_SCRUBS;i++) { //perform a set number of scrubs
		system("cat zynq_sys_wrapper.bit > /dev/xdevcfg");
		sleep(1);
	}
	
	done = 1;
	if(pthread_join(scrub_thread, NULL))
	{
		fprintf(stderr, "Error joining the pow monitor thread and scrubber.\n");
		return 2;
	}
	
	set_fdiv_value(&freq_scaler, 25);

	return 0;
}

