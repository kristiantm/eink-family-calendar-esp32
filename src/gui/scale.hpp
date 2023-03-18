#ifndef SCALE_HPP
#define SCALE_HPP
#include "Arduino.h"

class scale
{
    public:
        static int x_resMtp(double x, double x_res);
        static int y_resMtp(double y, double y_res);
};

#endif