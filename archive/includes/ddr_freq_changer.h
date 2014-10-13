//zynq device from software. 

//This was developed by Shane Fleming as work towards his PhD, Imperial College London.
//shane.fleming06@imperial.ac.uk

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "lock_count.h"

#define DDR_PLL_CTRL    0xf8000104
#define DDR_PLL_CFG     0xf8000114
#define DDR_PLL_STATUS      0xf800010c

#define DDR_FMAP_SIZE 4096UL
#define DDR_FMAP_MASK (FMAP_SIZE - 1)
#define DDR_CHECK_BIT(var,pos) !!((var) & (1<<(pos)))

typedef struct {
	void *mapped_dev_ctrl_reg;
	void *mapped_dev_cfg_reg;
	void *mapped_dev_status_reg;
	int fdiv;
} ddr_pll_data;


void setup_ddr_pll( arm_pll_data * pll_data);
void set_ddr_fdiv_value( arm_pll_data * pll_data, int fdiv);


/*
	This function maps the registers that are used to control the pll registers into the userspace program.
*/
void setup_ddr_pll( ddr_pll_data * pll_data)
{

void *mapped_ctrl_reg;
void *mapped_cfg_reg;
void *mapped_status_reg;
int memfd;

off_t cfg_reg = DDR_PLL_CFG;
off_t ctrl_reg = DDR_PLL_CTRL;
off_t status_reg = DDR_PLL_STATUS;

 memfd = open("/dev/mem", O_RDWR | O_SYNC); //to open this the program needs to be run as root
        if (memfd == -1) {
        printf("Can't open /dev/mem.\n");
        return -1;
    }
 //printf("/dev/mem opened.\n");

    // Map one page of memory into user space such that the device is in that page, but it may not
    // be at the start of the page.

//-------------ARM_PLL_CTRL-----------------------------------------------------------------------
mapped_ctrl_reg = mmap(0, DDR_FMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, ctrl_reg & ~DDR_FMAP_MASK);
    if (mapped_ctrl_reg == (void *) -1) {
        printf("Can't map the memory to user space.\n");
        return -1;
    }
     //printf("Memory mapped at address %p.\n", mapped_ctrl_reg);

pll_data->mapped_dev_ctrl_reg = mapped_ctrl_reg + (ctrl_reg & DDR_FMAP_MASK); //Our register is now at location mapped_dev_ctrl_reg in userspace
//---------------------------------------------------------------------------------------------

//-----------------ARM_PLL_CFG--------------------------------------------------------------------
mapped_cfg_reg = mmap(0, DDR_FMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, cfg_reg & ~DDR_FMAP_MASK);
        if(mapped_cfg_reg == (void *) -1) {
                printf("Can't map the memory to user space.\n");
                return -1;
        }
//printf("Memory mapped at address %p. \n", mapped_cfg_reg);
pll_data->mapped_dev_cfg_reg = mapped_cfg_reg + (cfg_reg & DDR_FMAP_MASK);
//------------------------------------------------------------------------------------------------

//---------------PLL_STATUS-----------------------------------------------------------------------
mapped_status_reg = mmap(0, DDR_FMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, status_reg & ~DDR_FMAP_MASK);
        if(mapped_status_reg == (void *) -1) {
                printf("Can't map the memory to user space. \n");
                return -1;
        }
//printf("Memory mapped at address %p. \n", mapped_status_reg);
pll_data->mapped_dev_status_reg = mapped_status_reg + (status_reg & DDR_FMAP_MASK);
//------------------------------------------------------------------------------------------------

}

void set_fdiv_value( ddr_pll_data * pll_data, int input_divider)
{

struct PLL_PARAM current_parameters; //declare the structures for the current pll_parameters
current_parameters = fdiv_params[input_divider - 13]; //find the appropriate parameters from the structure offset by 13

//--------SETUP FDIV, LOCK_CNT, PLL_CP, PLL_RES---------------------------------
int cfg_val = *((volatile unsigned long *) pll_data->mapped_dev_cfg_reg);
int ctrl_val = *((volatile unsigned long *) pll_data->mapped_dev_ctrl_reg);

//printf("Initial values\n CTRL= %d\n CFG= %d\n", ctrl_val, cfg_val);

*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) &= ~(63 << 12); //clear the bits for fdiv [18:12]
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) |= (input_divider << 12); //Set the fdiv value

*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) &= ~(1023 << 12); //Clear the LOCK_CNT bits [21:12]
*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) |= (current_parameters.LOCK_CNT << 12); //Set the LOCK_CNT

*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) &= ~(15 << 8); //Clear the bits of PLL_CP [11:8]
*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) |= (current_parameters.PLL_CP << 8); //Assign the value of PLL_CP

*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) &= ~(15 << 4); //Clear the bits of PLL_RES [7:4]
*((volatile unsigned long *)pll_data->mapped_dev_cfg_reg) |= (current_parameters.PLL_RES << 4); //Assign the value for PLL_RES 

cfg_val = *((volatile unsigned long *) pll_data->mapped_dev_cfg_reg);
ctrl_val = *((volatile unsigned long *) pll_data->mapped_dev_ctrl_reg);

//printf("Finished setting up the PLL, read for reset\n CTRL= %d\n CFG= %d\n", ctrl_val, cfg_val);
//------------------------------------------------------------------------------

//-------------BYPASS the PLL --------------------------------------------------
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) |= (1 << 4); //set bit 4 to enable bypass mode
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) &= ~(1 << 3); //clear PLL_BYPASS_QUAL bit
ctrl_val = *((volatile unsigned long *) pll_data->mapped_dev_ctrl_reg);
//printf("Bypass setup: CTRL= %d\n", ctrl_val);

//------------------------------------------------------------------------------

//------------PLACE PLL IN RESET MODE-------------------------------------------
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) |= (1 << 0); //set bit 0 to 1 for reset
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) &= ~(1 << 0); //set bit 0 to 0 to bring out of reset
//------------------------------------------------------------------------------

//----------Wait for clock stabilisation----------------------------------------
while( DDR_CHECK_BIT(*((volatile unsigned long *)pll_data->mapped_dev_status_reg),0) ) {  }
//------------------------------------------------------------------------------

//----------Disable the PLL bypass----------------------------------------------
*((volatile unsigned long *)pll_data->mapped_dev_ctrl_reg) &= ~(1 << 4); //set bit 4 to zero
ctrl_val = *((volatile unsigned long *) pll_data->mapped_dev_ctrl_reg);
//printf("Bypass disabled: CTRL=%d\n", ctrl_val);
//------------------------------------------------------------------------------

pll_data->fdiv = input_divider;

}



