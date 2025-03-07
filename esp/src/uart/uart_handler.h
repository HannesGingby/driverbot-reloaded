#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

extern SoftwareSerial serial;

void setupUART();
void processUART();
void maixLed();

#endif // UART_HANDLER_H
