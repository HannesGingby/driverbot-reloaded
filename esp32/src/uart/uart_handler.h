#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <Arduino.h>

#define RX_PIN 4
#define TX_PIN 5

void setupUART();
void processUART();
void sendPosition(int tileX, int tileY);
void sendCommand(String command, int startPosX, int startPosY, int mapSizeX, int mapSizeY);
void maixLed();

#endif // UART_HANDLER_H
