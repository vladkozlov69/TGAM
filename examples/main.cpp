#include <Arduino.h>
#include "TGAM.h"

UART u1(p0, p1);

TGAM tgam(&u1, &SerialUSB);

void setup() 
{
    SerialUSB.begin(9600);
    u1.begin(9600);
}

void loop() 
{
    if (tgam.read())
    {
        tgam.dumpPayload();
    }
}
