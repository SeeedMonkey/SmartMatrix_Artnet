
#include <Wire.h>
#include <SmartMatrix3.h>

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

										  // Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 0;

byte mac[] = {
  0x4A, 0x15, 0xC9, 0x75, 0xD5, 0x88
};
byte ip[] = {192, 168, 2, 106};


uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;

void setup() {
   artnet.begin(mac, ip);
   artnet.setArtDmxCallback(onDmxFrame); //gets called if a frame comnplete
  Serial.begin(9600);
  delay(200);
 
  // setup matrix
  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(defaultBrightness);
 }

void loop() {
 
	artnet.read();
}
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
	sendFrame = 1;
	// Store which universe has got in
	if (universe < maxUniverses)
		universesReceived[universe] = 1;

	for (int i = 0; i < maxUniverses; i++)
	{
		if (universesReceived[i] == 0)
		{
			//Serial.println("Broke");
			sendFrame = 0;
			break;
		}
	}

	// read universe and put into the right part of the display buffer
	for (int i = 0; i < length; i++)
	{
		int bufferIndex = i + ((universe - startUniverse) * length);
		if (bufferIndex < numberOfChannels) // to verify
			channelBuffer[bufferIndex] = byte(data[i]);
	}
	int dot = 0;
	int row = 0;
	
	// send to leds	
	for (int ii = 0; ii <3072; ii++) { // 6 universes of 510 channels each (170 pixels *3 colors) plus 12 channels on universe 7
		if (dot>31) { //if one row is full
			dot = 0;
			row += 1;
		}
		if (ii <= 169) { //first universe
			r = channelBuffer[((ii) * 3)];
			 g = channelBuffer[((ii) * 3) + 1];
			 b = channelBuffer[((ii) * 3) + 2];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		else if (ii > 169 && ii <= 339) { // second universe, offset of two channels to avoid one pixel being split between two universes
			 r = channelBuffer[(ii * 3) + 2];
			 g = channelBuffer[(ii * 3) + 3];
			 b = channelBuffer[(ii * 3) + 4];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
			}
		else if (ii > 339 && ii <= 509) {//	third universe, offset of four channels to avoid one pixel being split between two universes
			 r = channelBuffer[(ii * 3) + 4];
			 g = channelBuffer[(ii * 3) + 5];
			 b = channelBuffer[(ii * 3) + 6];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		else if (ii > 509 && ii <= 679) { // see above
			r = channelBuffer[(ii * 3) + 6];
			g = channelBuffer[(ii * 3) + 7];
			b = channelBuffer[(ii * 3) + 8];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		else if (ii > 679 && ii <= 849) {
			r = channelBuffer[(ii * 3) + 8];
			g = channelBuffer[(ii * 3) + 9];
			b = channelBuffer[(ii * 3) + 10];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		else if (ii > 849 && ii <= 1019) {
			r = channelBuffer[(ii * 3) + 10];
			g = channelBuffer[(ii * 3) + 11];
			b = channelBuffer[(ii * 3) + 12];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		else if (ii > 1019 ) {
			r = channelBuffer[((ii) * 3) + 12];
			g = channelBuffer[((ii) * 3) + 13];
			b = channelBuffer[((ii) * 3) + 14];
			rgb24 pixel = { r,g,b };
			backgroundLayer.drawPixel(dot, row, pixel);
		}
		
		
		
		
		dot += 1; // go to the next Channel
	}
	backgroundLayer.swapBuffers(); //Display the loaded frame
	sendFrame = 0;
	if (sendFrame)
	{
		sendFrame = 0;
		// Reset universeReceived to 0
		memset(universesReceived, 0, maxUniverses);
	}
}

