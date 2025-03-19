#ifndef CONFIG_H
#define CONFIG_H

#include <WiFi.h>
#include "time.h"
#include <TM1637Display.h>
#include <Keypad.h>
#include <set>
#include <WebServer.h>
#include <Arduino.h>

const uint8_t SEG_ADD[] = {
	SEG_A | SEG_B | SEG_C | SEG_E| SEG_F | SEG_G,    
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           
  0x00
	},
  SEG_DEL[] = {
	  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           
	  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           
    SEG_D | SEG_E | SEG_F,                           
    0x00
  },
  SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   
    SEG_C | SEG_E | SEG_G,                           
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            
    };
  
#endif