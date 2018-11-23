/*
 * HLK_M35.h
 *
 *  Created on: Sep 2, 2016
 *      Author: Afshin Alaghehband
 *      AfshinAlaghehband@gmail.com
 */
#include "HLK_M35.h"
#include <stdio.h>
#include "stm32f1xx.h"
/**********************************************************
 * hlk_init
 *  initial HLK
 *  @param  hlk Pointer to a hlk typedef that use in all other function
 *  @param  default_Port Pointer to a GPIO Port of HLK default
 *  @param  default_Pin number of GPIO pin of HLK default
 *  @param  reset_Port Pointer to a GPIO Port of HLK default
 *  @param  reset_Pin number of GPIO pin of HLK default
 **********************************************************/
void hlk_init(HLK_t* hlk, GPIO_TypeDef* def_Port, uint16_t def_Pin,
		GPIO_TypeDef* rst_Port, uint16_t rst_Pin, UART_HandleTypeDef *uart_t) {

	hlk->default_Port = def_Port;
	hlk->default_Pin = def_Pin;
	hlk->reset_Port = rst_Port;
	hlk->reset_Pin = rst_Pin;
	hlk->uart_t = uart_t;
	HAL_GPIO_WritePin(hlk->reset_Port, hlk->reset_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(hlk->default_Port, hlk->default_Pin, GPIO_PIN_SET);
}
/**********************************************************
 * setAtCommandMode
 *  change HLK mode to at command
 *  @param  GPIO_Port Pointer to a GPIO Port of HLK Defult
 *  @param  GPIO_Pin number of GPIO pin of HLK Defult
 **********************************************************/
void hlk_setAtCommandMode(HLK_t* hlk) {
	HAL_GPIO_WritePin(hlk->default_Port, hlk->default_Pin, GPIO_PIN_RESET);
	HAL_Delay(LONG_DELAY);
	HAL_GPIO_WritePin(hlk->default_Port, hlk->default_Pin, GPIO_PIN_SET);
	HAL_Delay(SHORT_DELAY);

}

/**********************************************************
 * setWifiNetWorkMode
 *  Set AP or STA netWork mode
 *  @param  uart_t Pointer to a UART_HandleTypeDef structure that contains
 *  @param  mode use 1 for AP mode and other for STA mode
 **********************************************************/
void hlk_setWifiNetWorkMode(HLK_t* hlk, uint8_t mode) {
	if (mode == 1)
		HAL_UART_Transmit(hlk->uart_t, "at+WA=1\r\n", 9, 100); //AP mode
	else
		HAL_UART_Transmit(hlk->uart_t, "at+WA=2\r\n", 9, 100); //STA mode --Connect to wi-fi
	HAL_Delay(SHORT_DELAY);
}

/**********************************************************
 * setSoftApSSIDName
 *  Set Shared Wifi Name
 *  @param  uart_t Pointer to a UART_HandleTypeDef structure that contains
 *  @param  name pointer to string name
 **********************************************************/
void hlk_setSoftApSSIDName(HLK_t* hlk, char *name) {
	char ch[30];
	sprintf(ch, "at+Assidl=%d\r\n", strlen(name));
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); //ssid name len
	HAL_Delay(SHORT_DELAY);
	sprintf(ch, "at+Assid=%s\r\n", name);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // ssid name
	HAL_Delay(SHORT_DELAY);
}

/**********************************************************
 * setEncrepionMode
 *  Set Password in SoftAp Mode
 *  @param  uart_t Pointer to a UART_HandleTypeDef structure that contains
 *  @param  mode Encrepion Mode Number ,(4 for Wap_aes/Wap_tkip ,
 *  		7 for Wpa2_aes/Wpa2_tkip ,9 for Wpa/Wpa2_tkip), Default is 7
 **********************************************************/
void hlk_setEncrepionMode(HLK_t* hlk, uint8_t mode) {
	HAL_UART_Transmit(hlk->uart_t, "at+Aam=9\r\n", sizeof("at+Aam=9\r\n"), 100);
	HAL_Delay(SHORT_DELAY);
}

/**********************************************************
 * setSoftApWpa2Key
 *  Set Shared Wifi password
 *  @param  uart_t Pointer to a UART_HandleTypeDef structure that contains
 *  @param  pass pointer to string password
 **********************************************************/
void hlk_setSoftApWpa2Key(HLK_t* hlk, char *pass) {
	char ch[30];
	sprintf(ch, "at+Apwl=%d\r\n", strlen(pass));
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); //set ssid password len
	HAL_Delay(SHORT_DELAY);
	sprintf(ch, "at+Apw=%s\r\n", pass);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); //set ssid password
	HAL_Delay(SHORT_DELAY);

}

//new commands

/**********************************************************
 * hlk_setRemoteIp
 *  Set hlk remot IP
 *  @param  hlk Pointer to a HLK TypeDef structure that contains
 *  @param  ip pointer to string of IP -> exp "192.168.1.1"
 **********************************************************/
void hlk_setRemoteIp(HLK_t* hlk, char *ip) {
	char ch[30];
	sprintf(ch, "at+UIp=%s\r\n", ip);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // ssid ip
	HAL_Delay(SHORT_DELAY);
}
/**********************************************************
 * hlk_hlk_setRemotePortsetIp
 *  Set hlk loacal port
 *  @param  hlk Pointer to a HLK TypeDef structure that contains
 *  @param  port int of port number -> exp 8080
 *  work at tcp server or udp server
 **********************************************************/
void hlk_setRemotePort(HLK_t* hlk, int port) {
	char ch[30];
	sprintf(ch, "at+ULport=%d\r\n", port);
		HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // local port
		HAL_Delay(SHORT_DELAY);
		sprintf(ch, "at+URport=%d\r\n", port);
			HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // local port
			HAL_Delay(SHORT_DELAY);
}

/**********************************************************
 * hlk_setNetworkType
 *  Set hlk network protocol type
 *  @param  hlk Pointer to a HLK TypeDef structure that contains
 *  @param  type number of protocol -> 0=nonoe ,1=Tcp server ,2=Tcp client ,3=udp server ,4=udp client
 **********************************************************/
void hlk_setNetworkProtocolType(HLK_t* hlk, char type) {
	char ch[30];
	sprintf(ch, "at+UType=%c\r\n", type);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // local port
	HAL_Delay(SHORT_DELAY);
}

void hlk_setNetworkParameter(HLK_t* hlk, char *ip, char *mask, char *gateWay,
		char *dns) {
	char ch[40];
	HAL_UART_Transmit(hlk->uart_t, "at+dhcp=0\r\n", strlen("at+dhcp=0\r\n"),
			100); // static dhcp

	sprintf(ch, "at+ip=%s\r\n", ip);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // ip
	HAL_Delay(SHORT_DELAY);
	sprintf(ch, "at+mask=%s\r\n", mask);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // mask
	HAL_Delay(SHORT_DELAY);
	sprintf(ch, "at+dns=%s\r\n", gateWay);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // geteWay
	HAL_Delay(SHORT_DELAY);
	sprintf(ch, "at+gw=%s\r\n", dns);
	HAL_UART_Transmit(hlk->uart_t, ch, strlen(ch), 100); // dns
	HAL_Delay(SHORT_DELAY);
}

/*
 *
 *
 *
 */

/**********************************************************
 * setSoftApWpa2Key
 *  save changed setting
 **********************************************************/
void hlk_saveConfiguration(HLK_t* hlk) {
	HAL_UART_Transmit(hlk->uart_t, "at+TS=1\r\n", sizeof("at+TS=1\r\n"), 100);
	HAL_Delay(LONG_DELAY);
	HAL_GPIO_WritePin(hlk->reset_Port, hlk->reset_Pin, GPIO_PIN_RESET);
	HAL_Delay(SHORT_DELAY);
	HAL_GPIO_WritePin(hlk->reset_Port, hlk->reset_Pin, GPIO_PIN_SET);
	HAL_Delay(SHORT_DELAY);
}

