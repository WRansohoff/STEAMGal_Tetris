# Overview

This is firmware for a 'tetris' game on a 'STEAMGal' board driven by an STM32 chip.

# Current Status

Incomplete. The firmware initializes the system clock to 48MHz driven by the HSI oscillator, then draws a simple menu to the OLED screen.

It sets up hardware interrupts for each of the 6 buttons - the 'A' button selects the test menu's start menu to start the game, and the 'Up' button returns to the menu from within the game. I'm hoping to add a 'pause' menu, though.

The onboard LED blinks on and off each game 'tick', which causes the current block to drop if it can, and fix in place on the grid if not. A 'game over' happens when a brick gets fixed in place while part of it is above the top line. Rows are cleared if necessary when a brick is fixed in place.

But there's no scoring, the game doesn't get faster as it progresses, there's no "next brick" display, etc. Just the basics.

Currently, only the STM32F051K8 is supported, but I hope to add the STM32F303K8 as well if time permits.

# Board Design

The board design is available under the 'board\_design\_v0' directory. It includes a circuit for 30-pin 128x64 SSD1306 monochrome OLED screens which seems to work, although I did crib that from the cheap 4-pin screen modules you can get off of ebay/aliexpress/etc for a few bucks. I've gotten several slightly different boards like that, and they all seem to use roughly the same design. Some of them omit the diode in the reset circuit, but since the screen has an onboard charge pump to generate a display bias voltage higher than 3.3V, I figure better safe than sorry.

Here's the first version of the board as rendered by OshPark. It seems to work as expected, but the ribbon connection is just barely close enough to the cutout to fold the screen over; I'll be moving that a bit in the next revision, along with adding an EEPROM chip. Also, the buzzer is sort of loud; I should probably add a potentiometer for volume control.

![STEAMGal\_V0](https://raw.githubusercontent.com/WRansohoff/STEAMGal_Firmware_test/master/board_design_v0/board_v0_render.png)
