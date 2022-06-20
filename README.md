# iot-calendar-client

Forked from [kristiantm/eink-family-calendar-esp32](https://github.com/kristiantm/eink-family-calendar-esp32), which details the hardware specs for this project.

This fork is intended for use with [iot-calendar-server](https://github.com/clockspot/iot-calendar-server).

At this writing (as my ESP32 hasn't come in yet), I'm using an Arduino Nano 33 IoT as a test bed. `/nano-test-local` is meant to pull NWS (JSON) and iCal data and process it locally, but for now I have set this aside in favor of `/nano-test-remote`, which leaves the hard work to [iot-calendar-server](https://github.com/clockspot/iot-calendar-server) and will simply render the data on the e-ink screen.