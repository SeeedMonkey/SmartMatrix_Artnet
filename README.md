# SmartMatrix_Artnet

A basic Artnet implimentation for a 32x32 Panel. WORK IN PROGRESS

Uses the Artnet.h library in combination with the SmartMatrix library to display Artnet DMX data to a 32x32 pixel Panel.

To recive the Artnet Data, i use a WS5200 Ethernet chip on a WIZ820i Module.
In order to make the Module work with the SmartMatrix shield on one Teensy, it is nessesarry to change the RESET and CS Pin on the WIZ820i module in the WS5100.ccp in C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries\Ethernet to different pins, i used 24 and 28 at the back of the teensy board. I also used a extra 3.3V Voltage converter for the module, since there can be pretty power hungry...
