#ifndef SCREEN_LIB
#include <Arduino.h>
#include "TFT_eSPI.h"

extern TFT_eSPI tft;
extern String scene;
// TFT_eSPI tft;  // Invoke library
#define TFT_GREY 0x5AEB // New colour

//audio mute
extern bool MUTE_MIC;
extern bool MUTE_AMP;
extern bool TALK;
extern bool LISTEN;

//cal data
#define REPEAT_CAL false
#define CALIBRATION_FILE "/TouchCalData3"

// Switch position and size
#define FRAME_X 85
#define FRAME_Y 64
#define FRAME_W 150
#define FRAME_H 50

#define FRAME_X1 85
#define FRAME_Y1 128
#define FRAME_W1 150
#define FRAME_H1 50

#define FRAME_X2 25
#define FRAME_Y2 128
#define FRAME_W2 100
#define FRAME_H2 50

#define FRAME_X3 185
#define FRAME_Y3 128
#define FRAME_W3 100
#define FRAME_H3 50

#define FRAME_X4 25
#define FRAME_Y4 64
#define FRAME_W4 100
#define FRAME_H4 50

#define FRAME_X5 185
#define FRAME_Y5 64
#define FRAME_W5 100
#define FRAME_H5 50

#define FRAME_X6 85
#define FRAME_Y6 64
#define FRAME_W6 150
#define FRAME_H6 50

#define FRAME_X7 85
#define FRAME_Y7 128
#define FRAME_W7 150
#define FRAME_H7 50

// Red zone size
#define STARTCALLBUTTON_X FRAME_X
#define STARTCALLBUTTON_Y FRAME_Y
#define STARTCALLBUTTON_W FRAME_W
#define STARTCALLBUTTON_H FRAME_H

#define WIFISETUPBUTTON_X FRAME_X1
#define WIFISETUPBUTTON_Y FRAME_Y1
#define WIFISETUPBUTTON_W FRAME_W1
#define WIFISETUPBUTTON_H FRAME_H1

#define MUTEMICBUTTON_X FRAME_X2
#define MUTEMICBUTTON_Y FRAME_Y2
#define MUTEMICBUTTON_W FRAME_W2
#define MUTEMICBUTTON_H FRAME_H2

#define LISTENBUTTON_X FRAME_X3
#define LISTENBUTTON_Y FRAME_Y3
#define LISTENBUTTON_W FRAME_W3
#define LISTENBUTTON_H FRAME_H3

#define CLOSECALLBUTTON_X FRAME_X4
#define CLOSECALLBUTTON_Y FRAME_Y4
#define CLOSECALLBUTTON_W FRAME_W4
#define CLOSECALLBUTTON_H FRAME_H4

#define MUTEAMPBUTTON_X FRAME_X5
#define MUTEAMPBUTTON_Y FRAME_Y5
#define MUTEAMPBUTTON_W FRAME_W5
#define MUTEAMPBUTTON_H FRAME_H5

#define BACKTOCALLBUTTON_X FRAME_X6
#define BACKTOCALLBUTTON_Y FRAME_Y6
#define BACKTOCALLBUTTON_W FRAME_W6
#define BACKTOCALLBUTTON_H FRAME_H6

#define BLOCKLISTENERBUTTON_X FRAME_X7
#define BLOCKLISTENERBUTTON_Y FRAME_Y7
#define BLOCKLISTENERBUTTON_W FRAME_W7
#define BLOCKLISTENERBUTTON_H FRAME_H7

void main_layout();
void wifi_setup_layout();
void open_call();
void listener_layout();
//void touch_calibrate();
void tft_setup();

#endif 