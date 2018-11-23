/*
 * SHT21.c
 *
 *  Created on: Aug 17, 2016
 *      Author: Afshin Alagheband
 *      Afshin.alpha@yahoo.com
 */
#include "SHT21.h"


/*********************************************************
 * @brief  Read an amount of data in blocking mode from a sensors register
 *
 * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 * @param  reg Internal register address
 */

uint16_t readSensor(I2C_HandleTypeDef *hi2c ,uint8_t reg) {
	uint8_t data[2] = { 0, 0 };
	uint16_t reData;
	HAL_I2C_Mem_Read(hi2c, SHT_ADDRESS << 1,reg, 1, data, 2, 100);
	reData = (uint16_t) data[0];
	reData = reData << 8;
	reData = reData | (uint16_t) data[1];
	reData &= ~0x0003;
	return reData;
}

/**********************************************************
 * GetTemperature
 *  Gets the current temperature from the sensor.
 *  @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *              the configuration information for the specified I2C.
 *  @param  hold control bit 'H' or 'h' for hold master mode and
 *  			'N' or 'n' for no hold master mode
 *  @return float - The temperature in Deg C
 **********************************************************/
float SHT21_GetTemperature(I2C_HandleTypeDef *hi2c,uint8_t hold)
{

	if(hold == 'n' || hold =='N')
		hold = TemperatureNoHold ;
	else if(hold == 'h' || hold =='H')
		hold =TemperatureHold ;
	else
		return 0 ;

	return (-46.85 + 175.72 / 65536.0 * (float)(readSensor(hi2c ,hold)));
}

/**********************************************************
 * GetHumidity
 *  Gets the current humidity from the sensor.
 *  @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
 *                the configuration information for the specified I2C.
 *  @param  hold control bit 'H' or 'h' for hold master mode and
 *  			'N' or 'n' for no hold master mode
 *  @return float - The relative humidity in %RH
 **********************************************************/
float SHT21_GetHumidity(I2C_HandleTypeDef *hi2c,uint8_t hold)
{
	if(hold == 'n' || hold =='N')
			hold = HumidityNoHold ;
		else if(hold == 'h' || hold =='H')
			hold = HumidityHold ;
		else
			return 0 ;
	return (-6.0 + 125.0 / 65536.0 * (float)(readSensor(hi2c ,hold)));
}

/**********************************************************
 * SoftReset
 *  Upon reception of this command, the sensor system reinitializes
 *		and starts operation according to the default settings.
 *
 *	@point The soft reset takes less than 15ms.
 **********************************************************/
void  SHT21_SoftReset(I2C_HandleTypeDef *hi2c){
	uint8_t data ;
	HAL_I2C_Mem_Read(hi2c, SHT_ADDRESS << 1,SoftReset, 1, data, 1, 100);
}








