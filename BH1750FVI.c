#include "BH1750FVI.h"

void BH1750FVI_Init(BH1750FVI_t *BH1750FVI, uint8_t adr, I2C_HandleTypeDef *i2c,
		uint8_t Mode) {

	// Set defaults for MTReg and Sensitivity
	BH1750FVI->MTReg = 69;
	BH1750FVI->Sensitivity = 1.00;
	BH1750FVI->Address = adr;
	BH1750FVI->i2c_t = i2c;
	// Power on the sensor
	PowerOn(BH1750FVI);
	// Set the mode
	SetMode(BH1750FVI, Mode);
}

void PowerOn(BH1750FVI_t *BH1750FVI) {
	I2CWrite(BH1750FVI,BH1750FVI_Power_On); // Power on the sensor
}

void Sleep(BH1750FVI_t *BH1750FVI) {
	I2CWrite(BH1750FVI,BH1750FVI_Power_Down); //Turn it off , Reset operator won't work in this mode
}

void Reset(BH1750FVI_t *BH1750FVI) {
	I2CWrite(BH1750FVI,BH1750FVI_Power_On); //Turn it on again
	I2CWrite(BH1750FVI,BH1750FVI_Reset_Sensor); //Reset

}

void SetAddress(BH1750FVI_t *BH1750FVI, uint8_t Addr) {

	// Set the address
	BH1750FVI->Address = Addr;

}

void SetMode(BH1750FVI_t *BH1750FVI, uint8_t Mode) {

	// Write the mode to the sensor
	I2CWrite(BH1750FVI,Mode);

	// Handle mode-specific changes
	// Note: delay for enough time to allow the sensor to get a measurement
	switch (Mode) {

	case BH1750FVI_Continuous_H:
		BH1750FVI->Resolution = 1;
		HAL_Delay(180);
		break;

	case BH1750FVI_Continuous_H2:
		BH1750FVI->Resolution = 0.5;
		HAL_Delay(180);
		break;

	case BH1750FVI_Continuous_L:
		BH1750FVI->Resolution = 1;
		HAL_Delay(24);
		break;

	case BH1750FVI_OneTime_H:
		BH1750FVI->Resolution = 1;
		HAL_Delay(180);
		break;

	case BH1750FVI_OneTime_H2:
		BH1750FVI->Resolution = 0.5;
		HAL_Delay(180);
		break;

	case BH1750FVI_OneTime_L:
		BH1750FVI->Resolution = 1;
		HAL_Delay(24);
		break;
	}
}

void SetMTReg(BH1750FVI_t *BH1750FVI, uint8_t MT) {

	// constrain MT to [31, 254]
	MT = min(max(MT, 31), 254);

	// Set the MTReg class variable so we can account for it while measuring
	BH1750FVI->MTReg = MT;

	// Shift the first 3 bytes of MT to the last 3
	// Then OR 64 (01000000) to add the 01000 prefix
	uint8_t highbyte = (MT >> 5) | 64;

	// Get rid of the first 3 bytes in MT by ANDing 31 (00011111)
	// Then OR 96 (01100000) to add the 011 prefix
	uint8_t lowbyte = (MT & 31) | 96;

	// Send the two MTReg bytes
	HAL_I2C_Master_Transmit(BH1750FVI->i2c_t, BH1750FVI->Address<< 1, &highbyte, 1,
			400);
	HAL_I2C_Master_Transmit(BH1750FVI->i2c_t, BH1750FVI->Address<< 1, &lowbyte, 1,
			400);

}

void SetSensitivity(BH1750FVI_t *BH1750FVI, float Sens) {

	// Convert sensitivity ratio to MT
	int MT = round(Sens * 69.0);

	// constrain MT to [31, 254]
	MT = min(max(MT, 31), 254);

	// Set the Sensitivity class variable by calculating the ratio
	// Note we use MT/69 here, which might be different from sens
	BH1750FVI->Sensitivity = MT / 69.0;

	// Set the Measurement Time to MT
	SetMTReg(BH1750FVI,MT);

}

float GetLux(BH1750FVI_t *BH1750FVI) {
	uint8_t data[2];
	uint16_t Intensity;
	float lux;

	// Ask the sensor for two bytes
	// Read the two bytes and OR them together for a 16 bit number

	HAL_I2C_Master_Receive(BH1750FVI->i2c_t,BH1750FVI->Address<< 1,data,2,400);

	Intensity = data[0];
	Intensity <<= 8;
	Intensity |= data[1];


	// Check to see if we've changed the sensitivity.
	if (BH1750FVI->Sensitivity != 1) {

		// According to datasheet, divide by 1.2 & multiply by Resolution
		// Sensitivity has been changed, so don't re-scale
		lux = Intensity / 1.2 * BH1750FVI->Resolution;

	} else {

		// According to datasheet, divide by 1.2 & multiply by Resolution
		// Rescale to account for Measurement Time change
		lux = (((Intensity / 1.2 )* BH1750FVI->Resolution )* 69.0 )/ BH1750FVI->MTReg;

	}

	// Return a float.  Only to 2 significant digits though
	return lux ;

}

void I2CWrite(BH1750FVI_t *BH1750FVI, uint8_t Opcode) {

	// Write a byte to the sensor
	HAL_I2C_Master_Transmit(BH1750FVI->i2c_t, BH1750FVI->Address << 1, &Opcode, 1,
			400);

}
