# KSensor

The code in this repository is used to run a Sparkfun Pro Micro 3.3v, a DS18B20 waterproof sensor, a RHT03 temperature sensor, and an ESP8266 WiFi module. The code is still a work in progress. Please don't hesitate to leave me comments or questions.

The code grabs readings from the sensors hooked up to the Pro Micro board and submits it to an endpoint over WiFi using the ESP8266 module.

The readings taken are air temperature, relative humidity, and water temperature (via a probe).

# Relay Sketch

This sketch is just to forward commands to an ESP8266 hooked up to some arduino board. This is done via a software serial interface in this instance, but it is highly recommended that you use a board with two hardware serial interfaces, or another means of relaying the command. This worked, but the commnad had to be sent over many times before it was received by the ESP8266 without error. The command needed to change the default baud rate to 9600 is in the baud_rate_command.txt file.

# Schematics

Eagle CAD was used to produce a schematic for the circuit, but has not been used to produce a board file. This will hopefully come at some point. It may help to reference the accompanying imgur album link to make sense of how the pins should be hooked up to the Pro Micro.

# License

This is licensed using the MIT license. Please do whatever you want with it.

# Imgur Album

https://imgur.com/gallery/qoYtd

This is a link to an album that offers a basic explanation of how this code was used in practice to produce a finished product.
