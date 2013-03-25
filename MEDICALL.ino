/*************************************************** 
  MEDICALL
  Version 2013/2/20

  1.Adafruit LED Matrix
  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x71, 0x72 or 0x73. 

  2.VS1053B Mp3+SD Shield
  
  
  3.Tracks Datasheet
  track001.mp3 ------ sound:power on //yes
  track002.mp3 ------ vocal:"hold on to your temple" 
  track003.mp3 ------ sound:start to scan the temple //yes
  track004.mp3 ------ vocal:"start to scan" // no
  track005.mp3 ------ sound:temple scanning //yes
  track006.mp3 ------ sound:temple scan finished //yes
  track007.mp3 ------ vocal:"nice job, next hold on to your chest"
  track008.mp3 ------ sound:start to scan the chest //yes
  track009.mp3 ------ vocal:"now,start to scan your chest"
  track010.mp3 ------ sound:chest scanning //yes
  track011.mp3 ------ sound:chest scan finished //yes
  track012.mp3 ------ vocal:"great,result will be proceeded" 
  track013.mp3 ------ sound:processing the result
  track014.mp3 ------ vocal:"You're dead" //yes
  track015.mp3 ------ vocal:"You're in danger" //yes
  track016.mp3 ------ vocal:"You're Okay" //yes
  track017.mp3 ------ sound:power off //yes
  track018.mp3 ------ sound:alarm //yes
  track019.mp3 ------ sound:"No action detected"
  track004.mp3 ------ extra sound:Dalek! //yes
 ****************************************************/

////////////////////////
//        libs        //
////////////////////////

/*********for IR **************/
#include <IRremote.h>

/*******for sound ************/
#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>

//and the MP3 Shield Library
#include <SFEMP3Shield.h>

/********for screen ***********/
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#define MATRIX_LEFT   0
#define MATRIX_MIDDLE 1
#define MATRIX_RIGHT  2


//MP3 sheild vars
SdFat sd;
SFEMP3Shield MP3player;

//lights vars
int brightness = 0;    // how bright the LED is
int fadeAmount = 25;    // how many points to fade the LED by

//screen vars
uint8_t counter = 0;

Adafruit_8x8matrix matrix[3];
const uint8_t PROGMEM matrixAddr[] = { 0x71, 0x72, 0x73 };

static uint8_t PROGMEM
  btImg[][24] = {                 // Mouth animation frame
//  { B00000000, B00000000, B00000000, // blank
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000,
//    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00001111,
    B00000000, B00000000, B00001111,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000011, B11001111,
    B00000000, B00000011, B11001111,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B11110011, B11001111,
    B00000000, B11110011, B11001111,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00111100, B11110011, B11001111,
    B00111100, B11110011, B11001111,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 }},
 tpImg[][24] = {   
//  { B00000000, B00011111, B00000000, // doctor 1
//    B00000000, B01101000, B10000000,
//    B00000011, B01100010, B01000000,
//    B00000100, B11100001, B01000000,
//    B00000100, B11101001, B01000000,
//    B00000011, B01100010, B01000000,
//    B00000000, B01100000, B10000000,
//    B00000000, B00011111, B00000000, },
  { B00000000, B00011111, B00000000, // doctor 2
    B00000000, B01100000, B10000000,
    B00000011, B01101010, B01000000,
    B00000100, B11100001, B01000000,
    B00000100, B11100001, B01000000,
    B00000011, B01101010, B01000000,
    B00000000, B01100000, B10000000,
    B00000000, B00011111, B00000000, }},
//  { B00000000, B00011111, B00000000, // doctor 3
//    B00000000, B01100000, B10000000,
//    B00000011, B01100010, B01000000,
//    B00000100, B11101001, B01000000,
//    B00000100, B11100001, B01000000,
//    B00000011, B01100010, B01000000,
//    B00000000, B01101000, B10000000,
//    B00000000, B00011111, B00000000, }},
  tcImg[][24] = {
  { B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },   
  { B00000000, B11000000, B00000000, // temple detected
    B00000000, B01100000, B00000000,
    B00000000, B00110000, B00000000,
    B00000000, B00011000, B00000000,
    B00000000, B00001100, B00000000,
    B00000000, B00000110, B00000000,
    B00000000, B00000011, B00000000,
    B00000000, B00000110, B00000000, },
  { B00000000, B00010000, B00000000, //start scanin
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000, }, // mark1
  { B00000000, B00011110, B00000000, // mark2
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000, },
  { B00000000, B00000011, B11000000, 
    B00000000, B00011110, B00000000, // mark3
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000, },
  { B00000000, B01111110, B00000000, 
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000, // mark4
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000, },
  { B00000001, B11000000, B00000000, 
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000,// mark5
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000, },
  { B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000,// mark6
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000, },
  { B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000,// mark7
    B00000000, B00010000, B00000000,
    B00000000, B00011100, B00000000,},
  { B00000000, B00011100, B00000000,
    B00000000, B00010000, B00000000,
    B00000000, B01110000, B00000000,
    B00000001, B11000000, B00000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000011, B11000000,
    B00000000, B00011110, B00000000,// mark8
    B00000000, B00010000, B00000000,}},
  ccImg[][24] = {   
  { B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00111110, B00000000,
    B00000000, B01111111, B00000000,
    B00000000, B00111110, B00000000,
    B00000001, B11111000, B00000000,
    B00000001, B11111000, B00000000,
    B00000000, B00111110, B00000000,// mark7
    B00000000, B01111111, B00000000,
    B00000000, B00111110, B00000000,},
  { B00000000, B00000000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00111110, B00000000,
    B00000000, B11111000, B00000000,
    B00000000, B11111000, B00000000,
    B00000000, B00111110, B00000000,// mark7
    B00000000, B00011100, B00000000,
    B00000000, B00000000, B00000000,},
  { B00000000, B00000000, B00000000,
    B00000000, B00001000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B01111000, B00000000,
    B00000000, B01111000, B00000000,
    B00000000, B00011100, B00000000,// f3
    B00000000, B00001000, B00000000,
    B00000000, B00000000, B00000000,},
  { B00000000, B00111110, B00000000,
    B00000000, B01111111, B00000000,
    B00000000, B00111110, B00000000,
    B00000001, B11111000, B00000000,
    B00000001, B11111000, B00000000,
    B00000000, B00111110, B00000000,// f4:same as f1
    B00000000, B01111111, B00000000,
    B00000000, B00111110, B00000000,},
  { B00000000, B00000000, B00000000,
    B00000000, B00001000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B01111000, B00000000,
    B00000000, B01111000, B00000000,
    B00000000, B00011100, B00000000,// f5:same as f3
    B00000000, B00001000, B00000000,
    B00000000, B00000000, B00000000,}},
  badImg[][24] = {
  { B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },   
  { B00000000, B00000111, B11000000, // you're dead
    B00001100, B00001000, B00100000,
    B00001100, B00010010, B01010000,
    B00111111, B00010000, B10010000,
    B00111111, B00010000, B10010000,
    B00001100, B00010010, B01010000,
    B00001100, B00001000, B00100000,
    B00000000, B00000111, B11000000 },
  { B00111110, B00000000, B00000000, // Temp 40
    B00100010, B00000000, B00111000,
    B00111110, B01111111, B11111100,
    B00000000, B01000011, B11111100,
    B00111110, B01000011, B11111100,
    B00001000, B01111111, B11111100,
    B00111000, B00000000, B00111000,
    B00000000, B00000000, B00000000, },
  { B00111110, B00001001, B00100000, // Sugar 1.8
    B00101010, B00000101, B01000000,
    B00111110, B00000011, B10000000,
    B00000000, B00011111, B11100000,
    B00000010, B00000011, B10000000,
    B00000010, B00000101, B01000000,
    B00111110, B00001001, B00100000,
    B00100010, B00000000, B00000000 },
  { B00111110, B00000010, B00000000, 
    B00101010, B00000011, B10000000,
    B00101010, B00000010, B00000000,
    B00000000, B00001110, B00000000,
    B00101110, B00111000, B00000000,
    B00101010, B00001111, B11000000,
    B00111010, B00000000, B01111000,
    B00000000, B00000011, B11000000, }, // pluse 53
  { B00000000, B00000111, B00000000, // heart dead
    B00011000, B00001111, B10000000,
    B00011000, B00001111, B11000000,
    B01111110, B00000111, B11100000,
    B01111110, B00000111, B11100000,
    B00011000, B00001111, B11000000,
    B00011000, B00001111, B10000000,
    B00000000, B00000111, B00000000, },
  { B00000000, B00000011, B11100000,
    B00011000, B00000111, B11110000,
    B00011000, B00000011, B11100000,
    B01111110, B00011111, B10000000,
    B01111110, B00011111, B10000000,
    B00011000, B00000011, B11100000,// lung dead
    B00011000, B00000111, B11110000,
    B00000000, B00000011, B11100000,}},
  ssImg[][24] = { 
  { B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, },  
  { B00000010, B01000111, B11000000, // you're soso
    B00000011, B01001000, B00100000,
    B00000010, B11010010, B01010000,
    B00000010, B01010000, B01010000,
    B00001000, B00010000, B01010000,
    B00001101, B00010010, B01010000,
    B00001011, B00001000, B00100000,
    B00001001, B00000111, B11000000, },
  { B00111110, B00000000, B00000000, // Temp 37
    B00100000, B00000000, B00111000,
    B00100000, B01111111, B11111100,
    B00000000, B01000011, B11111100,
    B00111110, B01000011, B11111100,
    B00101010, B01111111, B11111100,
    B00101010, B00000000, B00111000,
    B00000000, B00000000, B00000000, },
  { B00111110, B00001001, B00100000, // Sugar 4.9
    B00101010, B00000101, B01000000,
    B00111010, B00000011, B10000000,
    B00000000, B00011111, B11100000,
    B00000010, B00000011, B10000000,
    B00111110, B00000101, B01000000,
    B00001000, B00001001, B00100000,
    B00111000, B00000000, B00000000, },
  { B00101110, B00000010, B00000000, 
    B00101010, B00000011, B10000000,
    B00111110, B00000010, B00000000,
    B00000000, B00001110, B00000000,
    B00101110, B00111000, B00000000,
    B00101010, B00001111, B11000000,
    B00111110, B00000000, B01111000,
    B00000000, B00000011, B11000000, }, // pluse 66
  { B00000000, B00000111, B00000000, // heart dead
    B00011000, B00001111, B10000000,
    B00011000, B00001111, B11000000,
    B01111110, B00000111, B11100000,
    B01111110, B00000111, B11100000,
    B00011000, B00001111, B11000000,
    B00011000, B00001111, B10000000,
    B00000000, B00000111, B00000000, },
  { B00110000, B00000011, B11100000,
    B00011000, B00000111, B11110000,
    B00001100, B00000011, B11100000,
    B00000110, B00011111, B10000000,
    B00000011, B00011111, B10000000,
    B00000001, B10000011, B11100000,// lung ok
    B00000011, B00000111, B11110000,
    B00000110, B00000011, B11100000,}},
   okImg[][24] = {   
   {B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, },
   {B00110000, B00000111, B11000000, // you're okay
    B00011000, B00001000, B00100000,
    B00001100, B00010010, B10010000,
    B00000110, B00010000, B01010000,
    B00000011, B00010000, B01010000,
    B00000001, B00010010, B10010000,
    B00000011, B00001000, B00100000,
    B00000110, B00000111, B11000000, },
  { B00111110, B00000000, B00000000, // Temp 38
    B00101010, B00000000, B00111000,
    B00111110, B01111111, B11111100,
    B00000000, B01000011, B11111100,
    B00111110, B01000011, B11111100,
    B00101010, B01111111, B11111100,
    B00101010, B00000000, B00111000,
    B00000000, B00000000, B00000000, },
  { B00111110, B00001001, B00100000, // Sugar 5.3
    B00101010, B00000101, B01000000,
    B00101010, B00000011, B10000000,
    B00000000, B00011111, B11100000,
    B00000010, B00000011, B10000000,
    B00101110, B00000101, B01000000,
    B00101010, B00001001, B00100000,
    B00111010, B00000000, B00000000 },
  { B00101110, B00000010, B00000000, 
    B00101010, B00000011, B10000000,
    B00111110, B00000010, B00000000,
    B00000000, B00001110, B00000000,
    B00101110, B00111000, B00000000,
    B00101010, B00001111, B11000000,
    B00111110, B00000000, B01111000,
    B00000000, B00000011, B11000000, }, // pluse 66
  { B00110000, B00000111, B00000000, // heart ok
    B00011000, B00001111, B10000000,
    B00001100, B00001111, B11000000,
    B00000110, B00000111, B11100000,
    B00000011, B00000111, B11100000,
    B00000001, B10001111, B11000000,
    B00000011, B00001111, B10000000,
    B00000110, B00000111, B00000000, },
  { B00110000, B00000011, B11100000,
    B00011000, B00000111, B11110000,
    B00001100, B00000011, B11100000,
    B00000110, B00011111, B10000000,
    B00000011, B00011111, B10000000,
    B00000001, B10000011, B11100000,// lung ok
    B00000011, B00000111, B11110000,
    B00000110, B00000011, B11100000,}},
    
 amImg[][24] = {   
  { B00000000, B01111100, B00000000, // alarm
    B00000000, B10000010, B00000000,
    B00000001, B00000001, B00000000,
    B00000001, B11110001, B00000000,
    B00000001, B00010001, B00000000,
    B00000001, B00010001, B00000000,
    B00000000, B10000010, B00000000,
    B00000000, B01111100, B00000000, },
  { B00000000, B00000000, B00000000, // blank
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, }},

  dkImg[][24] = {   
  { B00000000, B11110000, B00000000, // doctor 1
    B00000001, B10111101, B00000000,
    B00000011, B10011101, B00000000,
    B00000011, B11111101, B00000000,
    B00000011, B11111101, B00000000,
    B00000011, B10011101, B00000000,
    B00000001, B10111101, B00000000,
    B00000000, B11110000, B00000000, },
  { B00000000, B11110000, B00000000, // doctor 1
    B00000001, B10111100, B10000000,
    B00000011, B10011100, B10000000,
    B00000011, B11111100, B10000000,
    B00000011, B11111100, B10000000,
    B00000011, B10011100, B10000000,
    B00000001, B10111100, B10000000,
    B00000000, B11110000, B00000000, }};
    
const uint8_t PROGMEM
  seq1[]  = { 1,1,2,2,3,3,3,255 }, //booting
  seq2[]  = { 0,0,0,0,0,0,0,0,0,0,0,255 }, //tpImg
  seq3[]  = { 1,1,255 }, //right mark
  seq4[]  = { 2,3,4,5,6,7,8,2,3,4,5,6,7,8,2,3,4,5,6,7,8,2,3,4,5,6,7,8,2,3,4,5,6,7,8,255 },// mind wave
  seq5[]  = { 1,1,1,2,2,2,3,3,5,2,2,2,1,1,4,2,2,2,3,3,5,2,2,2,1,1,4,2,2,2,3,3,5,2,2,2,1,1,1,255 }, //chest
  seq6[]  = { 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,1,255 },
  seq7[]  = { 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,1,255 },
  seq8[]  = { 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,1,255 },
  seq9[]  = { 0,0,1,1,0,0,1,1,0,0,1,1,255 }, //alarm
  seq10[]  = { 0,0,1,1,0,0,1,1,0,0,1,1,255}, //dalek
  *anim[] = { seq1,seq2,seq3,seq4,seq5,seq6,seq7,seq8,seq9,seq10,seq2,seq2};
  
uint8_t
  bootC     = 0,
  naC  = 0,
  tempT = 0,
  soundPos       = 0,
  pixelPos       =   0, // Current image number for image
  pixelCountdown =  10, // Countdown to next image change
  newPos         = 0, // New image position for current frame
  *seq,                 // Animation sequence currently being played back
  idx = 0;                  // Current array index within animation sequence

//IR vars
#define RECV_PIN  9
IRrecv irrecv(RECV_PIN);
decode_results remotes;
int last_command = 0;

//Mode control
//boolean modeDead = false;
boolean aWake = false;
boolean onT = false;
boolean onC = false;
boolean aLarm = false;
boolean Dalek = false;
boolean tLIGHT = false;
boolean bLIGHT = false;

void setup() {
  //Serial.begin(115200);
  //Serial.println("Starting...");
  
  //initialize lights
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);
  lightsOFF();
  
  //initialize screen
  for(uint8_t i=0; i<3; i++) {
    matrix[i] = Adafruit_8x8matrix();
    matrix[i].begin(pgm_read_byte(&matrixAddr[i]));
  }
  screenOFF();
  //Serial.println("Screen Initialized.");
  
  /********initiallize sound********/
  uint8_t result; //result code from some function as to be tested at later time.
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
  //Initialize the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    //Serial.println(F("Error happens when trying to start MP3 player"));
    if( result == 6 ) {
      //Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
    }
  }else{
    //Serial.println("Mp3 Player Initialized.");
  }
  
  //initiallize IR
  irrecv.enableIRIn(); // Start the receiver
  //Serial.println("IR Initialized.");
  
  
  delay(5000);//BITCH!give you some time to plugin/unplugin the cable ;)
 
}

void loop() {
  //keep IR sensor running and receving data
  if (irrecv.decode(&remotes)) {
    if(last_command!=remotes.value){
        Serial.println(remotes.value);
        irParse(remotes.value);
        last_command=remotes.value; 
     }
    irrecv.resume(); // Receive the next value
  }
  
  if(aWake){
     //first time boot
     if(bootC==0){
       //boot sound
       parseSound(1);
       //boot screen
       analogWrite(3,255);
       sDisplay(0);
       analogWrite(5,255);
       tLIGHT = true;
       bLIGHT = true;
       delay(2000);
       //lightsOFF();
       screenOFF();
       delay(2000);
       bootC++; //skip it for next time
       //delay(15000); //waiting for booting... no need
     }
       
     //waiting for the Temple Command   
     if(onT){
         if(tempT==0){
            //Serial.println("Starting Temple measurement");
            //turn off all rings
            parseSound(3); //nsT(); //notify scan starting 
            analogWrite(5,0);
            tLIGHT = false;
            analogWrite(3,0);
            bLIGHT = false;
            //sDisplay(2);///////
            delay(4000); 
            //parseSound(4); //confirmation "Now start to scan"
            //delay(5000);
            parseSound(5); //npT()//notofy scanning
            sDisplay(3);///////
            delay(5000);
            parseSound(6); //nfT(); //notify finishing
            delay(4000);
            tempT++;
         }
         if(onC){
              //Serial.println("Starting Chest measurement");
              parseSound(8); //nsC(); //notify scan starting 
              //sDisplay(2);///////
              
              analogWrite(5,255);
              tLIGHT = true;
              analogWrite(3,255);
              bLIGHT = true;
              
              delay(4000); 
              parseSound(9); //confirmation "Now start to scan"
              
              analogWrite(5,255);
              tLIGHT = true;
              analogWrite(3,255);
              bLIGHT = true;
              delay(5000);
              parseSound(10); //npC()//nottfy scanning
              sDisplay(4);///////light breathe with it
              delay(5000);
              parseSound(11); //nfC(); //notify finishing
              analogWrite(5,255);
              tLIGHT = true;
              analogWrite(3,255);
              bLIGHT = true;
              delay(4000);
              
              //results
              parseSound(12); //vocal:"great,result will be proceeded"
              sDisplay(11);///////
              delay(6000);
              parseSound(13); // sound:processing the result
              delay(2500);
              //random pick one of three cases
              int r = random(0,3);
              switch(r){
                case 0:
                  parseSound(14);//bad
                  sDisplay(5);///////
                  delay(10000);
                  break;
                case 1:
                  parseSound(15);//so so
                  sDisplay(6);///////
                  delay(10000);
                  break;
                case 2:
                  parseSound(16);//okay
                  sDisplay(7);///////
                  delay(10000);
              }

              shutDown();
          }else{
                 //notification for heart
                 //keeping notify "Please hold to your chest" five times
                 
                analogWrite(5,0);
                tLIGHT = false;
                analogWrite(3,255);
                bLIGHT = true;
                  if(naC<6){
                     parseSound(7); //hNotify();
                     naC++;
                     sDisplay(10);
                     delay(5000); //wait until press onC
                   } else{
                     parseSound(19);// aborting vocal sound
                     screenOFF();
                     delay(4000); //waiting for sound to be proceeded
                     shutDown();
                   }
          }    
       
       } else {
             //notification for temple
             //keeping notify "Please hold to the temple" for five times
             //turn off the bottom ring
             analogWrite(3,0);
             bLIGHT = false;
             
             if(naC<6){
               parseSound(2); //tNotify();
               naC++;
               sDisplay(1);
               delay(5000); //wait until press onT
             } else{
               parseSound(19);// aborting vocal sound
               screenOFF();
               delay(4000); //waiting for sound to be proceeded
               shutDown();
             }
        }
 
  }else if(aLarm){
     parseSound(18);// alarm
     sDisplay(8);
     //delay(5000);
  }else if(Dalek){
     parseSound(4);// dalek
     sDisplay(9);
     //delay(5000);
  }else{
     deadMode();
  }

}

////////////////////////////
// Mode: deatMode()       //
////////////////////////////
void deadMode(){
  //lights off
  lightsOFF();
  //screen off
  screenOFF();
  //sound off
  //soundOFF();
  //energy saver
  //energySaver();
  //won't turn off IR sensor :)
}

void lightsOFF(){
    analogWrite(5,0);
    analogWrite(3,0);
    tLIGHT = false;
    bLIGHT = false;
}

void screenOFF(){
  for(uint8_t i=0; i<3; i++) 
    {
      matrix[i].clear();
      matrix[i].writeDisplay();
    }
}


void shutDown(){
      parseSound(17);// go to sleep mode
      aWake = false;
      onC = false;
      onT = false;
      aLarm = false;
      Dalek = false;
      bootC = 0;
      naC = 0;
      tempT = 0;
      //delay(2000); //waiting for sound to be proceeded //no need 
      soundPos       = 0;
      pixelPos       =   0; // Current image number for image
      pixelCountdown =  10; // Countdown to next image change
      newPos         = 0; 
      lightsOFF();
}
////////////////////////////
// Mode: bootMode()       //
////////////////////////////

void lightsBoot(){
}
void sDisplay(uint8_t mode){ 
   idx=0;
   pixelPos       =   0; // Current image number for image
   newPos         = 0;
   pixelCountdown = 10;
   uint8_t bt = 255;
   boolean btW = false;
   
   seq  = (uint8_t *)pgm_read_word(&anim[mode]); 
   screenOFF();
   while(newPos != 255) { // Is the image in motion?
    if(--pixelCountdown == 0) { // Count down frames to next position
      //Serial.println(idx);
      if(mode==1){      
        if(tLIGHT){
          analogWrite(5,0);
          tLIGHT = false;
          //delay(1000);
        }else{
          //analogWrite(3,255);
          analogWrite(5,255);
          tLIGHT = true;
          //delay(1000);
        }
      }else if(mode==10){
        if(bLIGHT){
          analogWrite(3,0);
          bLIGHT = false;
          //delay(1000);
        }else{
          //analogWrite(3,255);
          analogWrite(3,255);
          bLIGHT = true;
          //delay(1000);
        }
      }else if(mode == 4){
         //analogWrite(5, 0);
           //Serial.println(brightness);
           //analogWrite(5, brightness);
           //analogWrite(3, brightness);
           //brightness = brightness + fadeAmount;
           //delay(30);
      }
      //delay(10);
      newPos = pgm_read_byte(&seq[idx++]); // New image position
      if(mode == 4){
          if(newPos ==4 || newPos ==5){
            fadeAmount = -fadeAmount ;
          }
          analogWrite(5, brightness);
          analogWrite(3, brightness); 
       }
      if(newPos == 255) { // End of list?
        //Serial.println(nwePos);
        if(idx-2<0){
          pixelPos = pgm_read_byte(&seq[0]);
        }else{
          pixelPos = pgm_read_byte(&seq[idx-2]);     // Yes, set image to neutral position
        }      
      } else {
        pixelPos       = newPos; // Set image to new position
        //pixelCountdown = pgm_read_byte(&seq[idx++]); // Read hold time
        pixelCountdown = 5;
      }
      
    }
    switch(mode){
      case 0:
        drawPixel(btImg[pixelPos]);
        break;
      case 1:
        drawPixel(tpImg[pixelPos]);
        break;
      case 2:
        drawPixel(tcImg[pixelPos]);
        break;
      case 3:
        drawPixel(tcImg[pixelPos]);
        break;
      case 4:
        drawPixel(ccImg[pixelPos]);
        break;
      case 5:
        drawPixel(badImg[pixelPos]);
        break;
      case 6:
        drawPixel(ssImg[pixelPos]);
        break;
      case 7:
        drawPixel(okImg[pixelPos]);
        break;
      case 8:
        drawPixel(amImg[pixelPos]);
        break;
      case 9:
        drawPixel(dkImg[pixelPos]);
        break;
      case 10:
        drawPixel(tpImg[pixelPos]);
        break;
      case 11:
        drawPixel(tpImg[pixelPos]);
        break;
    }
    updatePixel();
    delay(20); // could not delete it, for screen
  }
  
  if(newPos == 255){
      //analogWrite(3,255);
      if(mode==1){
        analogWrite(5,255);
        tLIGHT = true;
      }else if(mode==10){
        analogWrite(3,255);
        bLIGHT = true;
      }else if(mode==4){
        analogWrite(5,255);
        tLIGHT = true;
        analogWrite(3,255);
        bLIGHT = true;
      }
  }
}
/////////////////////////////
// Function: IR:: irParse  //
/////////////////////////////
void irParse(unsigned long command){
  if(command==16){
      //1 boot up
      aWake = true;
  }
  else if(command==2064){
      //2 start Temple
      onT = true;
      naC = 0;
  }
  else if(command==1040){
      //3 start Chest
      onC = true;
      naC = 0;
  }
  else if(command==2320){
      //O shut down
      shutDown();
  }
  else if(command==3088){
      //4 shut down
      aLarm = true;
  }
  else if(command==528){
      //5 shut down
      Dalek = true;
  }else{
      //blank
  }
}

////////////////////////////////////
// Function: Screen:: drawPixel   //
////////////////////////////////////
// Draw mouth image across three adjacent displays
void drawPixel(const uint8_t *img) {
  for(uint8_t i=0; i<3; i++) {
    matrix[MATRIX_LEFT + i].clear();
    matrix[MATRIX_LEFT + i].drawBitmap(i * -8, 0, img, 24, 8, LED_ON);
  }
}

////////////////////////////////////
// Function: Screen:: updatePixel //
////////////////////////////////////
//call it everytime!
void updatePixel(){
  // Refresh all matrices in one quick pass
  for(uint8_t i=0; i<3; i++) matrix[i].writeDisplay();
}


////////////////////////////////////
// Function: Sounds::playSound    //
////////////////////////////////////
//------------------------------------------------------------------------------
void parseSound(int key_command) {

 // uint8_t result; // result code from some function as to be tested at later time.
//
//  // Note these buffer may be desired to exist globably.
//  // but do take much space if only needed temporarily, hence they are here.
//  char title[30]; // buffer to contain the extract the Title from the current filehandles
//  char artist[30]; // buffer to contain the extract the artist name from the current filehandles
//  char album[30]; // buffer to contain the extract the album name from the current filehandles

  //Serial.print(F("Received command: "));
  //Serial.println(key_command);

  //if 0, stop the current track
  if(key_command == 0) {
    MP3player.stopTrack();
  //if 1-9, play corresponding track
  } else if(key_command >= 1 && key_command <= 19) {
    //convert ascii numbers to real numbers

    //tell the MP3 Shield to play a track
    //result = MP3player.playTrack(key_command);
    MP3player.playTrack(key_command);

    //check result, see readme for error codes.
//    if(result != 0) {
//      Serial.print(F("Error code: "));
//      Serial.print(result);
//      Serial.println(F(" when trying to play track"));
//    } else {
//
//      Serial.print(F("Playing: track"));
//      Serial.print(key_command);
//    }
  
  //if +/- to change volume
  } else if((key_command == 20) || (key_command == 21)) {
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t

    if(key_command == 20) { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(mp3_vol.byte[1] >= 254) { // range check
        mp3_vol.byte[1] = 254;
      } else {
        mp3_vol.byte[1] += 2; // keep it simpler with whole dB's
      }
    } else {
      if(mp3_vol.byte[1] <= 2) { // range check
        mp3_vol.byte[1] = 2;
      } else {
        mp3_vol.byte[1] -= 2;
      }
    }
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
//    Serial.print(F("Volume changed to -"));
//    Serial.print(mp3_vol.byte[1]>>1, 1);
//    Serial.println(F("[dB]"));
  }
}

