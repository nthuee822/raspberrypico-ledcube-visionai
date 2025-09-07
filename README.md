# raspberrypico-ledcube-visionai
An application of a Raspberry Pi Pico–based 8x8x8 LED cube with Grove Vision AI v2 module

## Introduction

There are many tutorials online about how to build an 8x8x8 LED Cube, but very few showcase practical applications. During an exhibition, we came across the Grove Vision AI V2 module, which inspired us to combine the two into a unique project.

Here’s how our system works: the camera on the Grove Vision AI V2 first recognizes a hand gesture (rock, paper, or scissors). It then sends the result via I²C to the Raspberry Pi Pico 2. The Pico 2 processes the data and communicates with the LED Cube’s PCB through the SPI interface. The PCB, equipped with eight shift registers, controls the LED Cube’s X–Y plane, while an additional perfboard manages the Z-axis. We will present the hardware setup in the next section.

This project is significant for two main reasons. First, we developed a Raspberry Pi Pico 2 library for the Grove Vision AI V2, making it easier for developers to integrate and leverage its vision features. Second, we created a generalized Raspberry Pi Pico library for the 8x8x8 LED Cube, allowing developers to easily modify display patterns simply by adjusting the code.

## Getting Started

<p align="center"> <img src="https://github.com/nthuee822/raspberrypico-ledcube-visionai/blob/main/src/120783.jpg" width="300"><br> figure 1. </p>

### Hardware setup: Grove vision AI v2 module

We recommend using the more powerful Grove Vision AI V2 for this project. Setup instructions can be found at the official guide: https://wiki.seeedstudio.com/grove_vision_ai_v2/.

The connection between the Grove Vision AI V2, Raspberry Pi Pico 2, and the PCB with eight shift registers is shown in figure 2.

<p align="center"> <img src="https://github.com/nthuee822/raspberrypico-ledcube-visionai/blob/main/src/hw1.png" width="500"><br> figure 2. </p>

### Hardware setup: 8x8x8 LED Cube

The four PCBs, each with eight shift registers, are connected as shown in figure 3.

<p align="center"> <img src="https://github.com/nthuee822/raspberrypico-ledcube-visionai/blob/main/src/hw2.png" width="300"><br> figure 3. </p>

The perfboard manages the Z-axis. Pins 6 to 13 on the Raspberry Pi Pico 2 are connected to eight 220 Ω resistors and eight NPN transistors, as shown in figure 4.

<p align="center"> <img src="https://github.com/nthuee822/raspberrypico-ledcube-visionai/blob/main/src/120782.jpg" width="300"><br> figure 4. </p>

## Software

### Grove vision AI v2 module
We revised the official Seeed_Arduino_SSCMA library (originally for Arduino) into an I²C interface library for the Raspberry Pi Pico.
This makes it easier to integrate the Grove Vision AI V2 with Pico-based projects.

### 8x8x8 LED Cube
We provide two versions of code for the LED Cube:

#### led_cube.c
A standalone project with several predefined special effects.
Simply run the code on the Raspberry Pi Pico, and it will light up the 8x8x8 LED Cube with built-in animations.

#### Cube_general.c
A more generalized implementation.
Developers can easily customize display effects by modifying only the matrix definition.

### Combination
Finally, we integrated everything into a rock–paper–scissors gesture recognition system.
The Grove Vision AI V2 recognizes the gesture, and the result is projected onto the 8x8x8 LED Cube.

For further details, please refer to Cube.cpp.

## Reference
