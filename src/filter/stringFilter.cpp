#include "stringFilter.hpp"

void stringFilter::YoungBoys_to_YB(String &message)
{
    if (message.indexOf("BSC Young Boys") || "BSC Young Boys -")
    {
        message.replace("BSC Young Boys", "YB");
    }
}

void stringFilter::umlauts(String &message)
{
    message.replace("ä", "ae");
    message.replace("ö", "oe");
    message.replace("ü", "ue");
}