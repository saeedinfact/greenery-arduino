#include "DS3231.h"

uint8_t bcd_to_decimal(uint8_t d) {
	return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}

uint8_t decimal_to_bcd(uint8_t d) {
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}

uint8_t DS3231_Read(uint8_t address) {
	uint8_t value = 0;
	HAL_I2C_Mem_Read(&DS3231_I2C, DS3231_Address << 1, address, 1, &value, 1,
			100);
	return value;
}

void DS3231_Write(uint8_t address, uint8_t value) {
	HAL_I2C_Mem_Write(&DS3231_I2C, DS3231_Address << 1, address, 1, &value, 1,
			100);

}

void DS3231_init() {
	DS3231_Write(controlREG, 0x00);
	DS3231_Write(statusREG, 0x08);
}

void DS3231_getTime(RTC_TimeTypeDef *time ,uint8_t *AM_PM, uint8_t hour_format) {
	uint8_t tmp = 0;

	tmp = DS3231_Read(secondREG);
	time->Seconds = bcd_to_decimal(tmp);
	tmp = DS3231_Read(minuteREG);
	time->Minutes = bcd_to_decimal(tmp);

	switch (hour_format) {
	case 1: {
		tmp = DS3231_Read(hourREG);
		tmp &= 0x20;
		*AM_PM = (uint8_t) (tmp >> 5);
		tmp = (0x1F & DS3231_Read(hourREG));
		time->Hours = bcd_to_decimal(tmp);
		break;
	}
	default: {
		tmp = (0x3F & DS3231_Read(hourREG));
		time->Hours = bcd_to_decimal(tmp);
		break;
	}
	}
}

void DS3231_getDate(RTC_DateTypeDef *date) {
	uint8_t tmp = 0;

	tmp = DS3231_Read(yearREG);
	date->Year = bcd_to_decimal(tmp);
	tmp = (0x1F & DS3231_Read(monthREG));
	date->Month = bcd_to_decimal(tmp);
	tmp = (0x3F & DS3231_Read(dateREG));
	date->Date = bcd_to_decimal(tmp);
	tmp = (0x07 & DS3231_Read(dayREG));
	date->WeekDay = bcd_to_decimal(tmp);
}

void DS3231_setTime(RTC_TimeTypeDef *time ,uint8_t am_pm_state, uint8_t hour_format) {
	uint8_t tmp = 0;

	DS3231_Write(secondREG, (decimal_to_bcd(time->Seconds)));
	DS3231_Write(minuteREG, (decimal_to_bcd(time->Minutes)));

	switch (hour_format) {
	case 1: {
		switch (am_pm_state) {
		case 1: {
			tmp = 0x60;
			break;
		}
		default: {
			tmp = 0x40;
			break;
		}
		}
		DS3231_Write(hourREG, ((tmp | (0x1F & (decimal_to_bcd(time->Hours))))));
		break;
	}

	default: {
		DS3231_Write(hourREG, (0x3F & (decimal_to_bcd(time->Hours))));
		break;
	}
	}
}

void DS3231_setDate(RTC_DateTypeDef *date) {
	DS3231_Write(dayREG, (decimal_to_bcd(date->WeekDay)));
	DS3231_Write(dateREG, (decimal_to_bcd(date->Date)));
	DS3231_Write(monthREG, (decimal_to_bcd(date->Month)));
	DS3231_Write(yearREG, (decimal_to_bcd(date->Year)));
}

void DS3231_setA1Time(uint8_t hSet, uint8_t mSet, uint8_t am_pm_state,
		uint8_t hour_format) {
	uint8_t tmp = 0;

	DS3231_Write(alarm1secREG, (0x7F & (decimal_to_bcd(0))));
	DS3231_Write(alarm1minREG, (0x7F & (decimal_to_bcd(mSet))));

	switch (hour_format) {
	case 1: {
		switch (am_pm_state) {
		case 1: {
			tmp = 0x60;
			break;
		}
		default: {
			tmp = 0x40;
			break;
		}
		}
		DS3231_Write(hourREG, ((tmp | (0x1F & (decimal_to_bcd(alarm1hrREG))))));
		break;
	}

	default: {
		DS3231_Write(hourREG, (0x3F & (decimal_to_bcd(alarm1hrREG))));
		break;
	}
	}
}

void DS3231_setA2Time(uint8_t hSet, uint8_t mSet, uint8_t am_pm_state,
		uint8_t hour_format) {
	uint8_t tmp = 0;

	DS3231_Write(alarm2minREG, (0x7F & (decimal_to_bcd(mSet))));

	switch (hour_format) {
	case 1: {
		switch (am_pm_state) {
		case 1: {
			tmp = 0x60;
			break;
		}
		default: {
			tmp = 0x40;
			break;
		}
		}
		DS3231_Write(hourREG, ((tmp | (0x1F & (decimal_to_bcd(alarm2hrREG))))));
		break;
	}

	default: {
		DS3231_Write(hourREG, (0x3F & (decimal_to_bcd(alarm2hrREG))));
		break;
	}
	}
}

void DS3231_getA1Time(uint8_t *p2, uint8_t *p1, uint8_t *p0,
		uint8_t hour_format) {
	uint8_t tmp = 0;

	tmp = DS3231_Read(alarm1minREG);
	*p1 = bcd_to_decimal(tmp);

	switch (hour_format) {
	case 1: {
		tmp = DS3231_Read(alarm1hrREG);
		tmp &= 0x20;
		*p0 = (uint8_t) (tmp >> 5);
		tmp = (0x1F & DS3231_Read(alarm1hrREG));
		*p2 = bcd_to_decimal(tmp);
		break;
	}
	default: {
		tmp = (0x3F & DS3231_Read(alarm1hrREG));
		*p2 = bcd_to_decimal(tmp);
		break;
	}
	}
}

void DS3231_getA2Time(uint8_t *p2, uint8_t *p1, uint8_t *p0,
		uint8_t hour_format) {
	uint8_t tmp = 0;

	tmp = DS3231_Read(alarm2minREG);
	*p1 = bcd_to_decimal(tmp);

	switch (hour_format) {
	case 1: {
		tmp = DS3231_Read(alarm2hrREG);
		tmp &= 0x20;
		*p0 = (uint8_t) (tmp >> 5);
		tmp = (0x1F & DS3231_Read(alarm2hrREG));
		*p2 = bcd_to_decimal(tmp);
		break;
	}
	default: {
		tmp = (0x3F & DS3231_Read(alarm2hrREG));
		*p2 = bcd_to_decimal(tmp);
		break;
	}
	}
}

float DS3231_getTemp() {
	float t = 0.0;
	uint8_t lowByte = 0;
	signed char highByte = 0;
	lowByte = DS3231_Read(tempLSBREG);
	highByte = DS3231_Read(tempMSBREG);
	lowByte >>= 6;
	lowByte &= 0x03;
	t = ((float) lowByte);
	t *= 0.25;
	t += highByte;
	return t;

}
