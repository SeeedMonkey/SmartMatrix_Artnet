// Basic sketch to display Artnet DMX pixeldata on a 32x32 pixel module via the SmartMatrix library.
// WORK IN PROGRESS!

//ARTNET.h : This example may be copied under the terms of the MIT license, see the LICENSE file for details
//ADAFRUIT_SLEEPYDOG: This library was used under the terms of the MIT licence
#include <Wire.h>
#include <SmartMatrix3.h>
#include <FastLED.h>
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness

#include <Artnet.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
Artnet artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = 3084; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels + 1]; // Combined universes into a single array
										
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);   // Check if we got all universes
bool universesReceived[maxUniverses];
bool sendFrame = 0;

byte mac[] = {0x4A, 0x15, 0xC9, 0x75, 0xD5, 0x88};
byte ip[] = {192, 168, 2, 106};

#include <Adafruit_SleepyDog.h>

//rgb24 *buffer = backgroundLayer.backBuffer(); Method 2, causes flicker
void setup() {
   artnet.begin(mac, ip);
   artnet.setArtDmxCallback(onDmxFrame); //gets called if a frame comnplete
  Serial.begin(115200);
  Serial.print("Start");

  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(defaultBrightness);
    
  Watchdog.enable(2000);
 }

void loop() {
 	artnet.read(); //check for new Artnet data
	Watchdog.reset();
}
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{	
	sendFrame = 1;
	if (universe < maxUniverses) universesReceived[universe] = 1; // Store which universe has got in
	/*		 //Alternative method, causes flicker in the display (Method 2)
	int section = universe * 170;
	if (universe < 6) {
			for (int i = 0; i < 170; i++) 
		{
			buffer[section + i] = CRGB(data[(i * 3)], data[(i * 3) + 1], data[(i * 3)+ 2]);
		}
	}
	else {
		for (int i = 0; i < 4; i++) 
		{
			buffer[section + i] = CRGB(data[(i * 3)], data[(i * 3) + 1], data[(i * 3) + 2]);
		}
	}
	*/
	for (int i = 0; i < length; i++) // Load data into the channelBuffer at the right spot (Method 1)
	{
		int bufferIndex = i + ((universe - startUniverse) * length);
		if (bufferIndex < numberOfChannels) // to verify
			channelBuffer[bufferIndex] = byte(data[i]);
	}
	
	for (int i = 0; i < maxUniverses; i++) //check if all universes are there
	{
		if (universesReceived[i] == 0)
		{
			sendFrame = 0;
			break;
		}
	}
	
	if (sendFrame) //if all universes are there, send frame to display
	{
		
		rgb24 *buffer = backgroundLayer.backBuffer(); //create an new buffer based on the backBuffer (method 1)
		while (backgroundLayer.isSwapPending());
		for (int ii = 0; ii <1024; ii++) { // go thru every pixel (Method 1) takes between 350 and 480 microseconds 
			if (ii <= 169) { //first universe
				buffer[ii] = CRGB(channelBuffer[((ii) * 3)], channelBuffer[((ii) * 3) + 1], channelBuffer[((ii) * 3) + 2]);
			}
			else if (ii > 169 && ii <= 339) { // second universe, offset of two channels to avoid one pixel being split between two universes
				buffer[ii] = CRGB(channelBuffer[(ii * 3) + 2], channelBuffer[(ii * 3) + 3], channelBuffer[(ii * 3) + 4]);
			}
			else if (ii > 339 && ii <= 509) {//	third universe, offset of four channels to avoid one pixel being split between two universes
				buffer[ii] = CRGB(channelBuffer[(ii * 3) + 4], channelBuffer[(ii * 3) + 5], channelBuffer[(ii * 3) + 6]);
			}
			else if (ii > 509 && ii <= 679) { // see above
				buffer[ii] = CRGB(channelBuffer[(ii * 3) + 6], channelBuffer[(ii * 3) + 7], channelBuffer[(ii * 3) + 8]);
			}
			else if (ii > 679 && ii <= 849) {
				buffer[ii] = CRGB(channelBuffer[(ii * 3) + 8], channelBuffer[(ii * 3) + 9], channelBuffer[(ii * 3) + 10]);
			}
			else if (ii > 849 && ii <= 1019) {
				buffer[ii] = CRGB(channelBuffer[(ii * 3) + 10], channelBuffer[(ii * 3) + 11], channelBuffer[(ii * 3) + 12]);
			}
			else if (ii > 1019) {
				buffer[ii] = CRGB(channelBuffer[((ii) * 3) + 12], channelBuffer[((ii) * 3) + 13], channelBuffer[((ii) * 3) + 14]);
			}
			
		}
		
		backgroundLayer.swapBuffers(false); //Display the loaded frame
		//buffer = backgroundLayer.backBuffer(); Method 2
		//matrix.countFPS();
		memset(universesReceived, 0, maxUniverses);// Reset universeReceived to 0		
	}
}
