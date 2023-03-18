#ifndef STRING_FILTER_HPP
#define STRING_FILTER_HPP
#include "Arduino.h"

class stringFilter
{
    public:
        static void YoungBoys_to_YB(String & message);
        static void umlauts(String & message);
};

#endif