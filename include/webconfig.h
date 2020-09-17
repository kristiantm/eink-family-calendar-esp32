#include <AutoConnect.h>


#define PARAM_FILE      "/elements.json"

static const char PAGE_ELEMENTS[] PROGMEM = R"(
{
  "uri": "/elements",
  "title": "Calendar",
  "menu": true,
  "element": [
    {
      "name": "text",
      "type": "ACText",
      "value": "Configuration for calendar",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "check",
      "type": "ACInput",
      "label": "Open Weather Map Key",
      "placeholder": "OWM Key"
    },
    {
      "name": "input",
      "type": "ACInput",
      "label": "Google Script API",
      "placeholder": "Google API"
    },
    {
      "name": "radio",
      "type": "ACInput",
      "label": "Lattitude",
      "placeholder": "Eg. 50.2"
    },
    {
      "name": "select",
      "type": "ACInput",
      "label": "Longitude",
      "placeholder": "Eg. 10.3"
    },
    {
      "name": "load",
      "type": "ACSubmit",
      "value": "Load",
      "uri": "/elements"
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save",
      "uri": "/save"
    },
    {
      "name": "adjust_width",
      "type": "ACElement",
      "value": "<script type=\"text/javascript\">window.onload=function(){var t=document.querySelectorAll(\"input[type='text']\");for(i=0;i<t.length;i++){var e=t[i].getAttribute(\"placeholder\");e&&t[i].setAttribute(\"size\",e.length*.8)}};</script>"
    }
  ]
}
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
  "uri": "/save",
  "title": "Elements",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "format": "Elements have been saved to %s",
      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
    },
    {
      "name": "validated",
      "type": "ACText",
      "style": "color:red"
    },
    {
      "name": "echo",
      "type": "ACText",
      "style": "font-family:monospace;font-size:small;white-space:pre;"
    },
    {
      "name": "ok",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/elements"
    }
  ]
}
)";

