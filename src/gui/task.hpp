#ifndef TASK_HPP
#define TASK_HPP
#include "Arduino.h"
#include "google/googleScriptRequest.hpp"

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>

#include "gui/scale.hpp"
#include "google/timeServer.hpp"
//#include "gui/e_paper_driver.hpp"

class task //: e_paper_driver
{
    public:
        //void draw(GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> &display);
        void updateCalendar(void);
        void drawCentreString(const char *buf, int x, int y);
    private:

};

#endif