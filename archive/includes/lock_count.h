//This header file contains the stucture for the PLL lock count settings
//It contains an array of records that can be referenced using the desired 
//f_div value. It then returns the appropriate PLL_CP, PLL_RES, and LOCK_CNT 
//values

//Created by Shane Fleming, Imperial college London
//Intended for use with freqeuncy_scaling on the zedboard

//Define the structure for each element
struct PLL_PARAM {
	int PLL_CP; //value to set the PLL charge pump control
	int PLL_RES; //value to se the PLL loop filter resistor control
	int LOCK_CNT; //Sets the number of lock cycles the PLL needs to have clkref and clkfb aligned
};

struct PLL_PARAM fdiv_params[] = 
{
 	{2,6,750},
	{2,6,700},
	{2,6,650},
	{2,10,625},
	{2,10,575},
	{2,10,550},
	{2,10,525},
	{2,12,500},
	{2,12,475},
	{2,12,450},
	{2,12,425},
	{2,12,400},
	{2,12,400},
	{2,12,375},
	{2,12,350},
	{2,12,350},
	{2,12,325},
	{2,12,325},
	{2,2,300},
	{2,2,300},
	{2,2,300},
	{2,2,275},
	{2,2,275},
	{2,2,275},
	{2,2,250},
	{2,2,250},
	{2,2,250},
	{2,2,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{3,12,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250},
	{2,4,250}
};
