#ifndef SHARP_IR_H
#define SHARP_IR_H

#include <Arduino.h>

class SharpIR
{
public:
    SharpIR(int analogPin);
    void init();
    int getDistance();

private:
    int analogPin;

    float analogToDistance(int analogValue);
};

#endif
