# raspberrypico-ledcube-visionai
An application of a Raspberry Pi Pico–based 8x8x8 LED cube with Grove Vision AI v2 module

## Introduction

There are many tutorials online about how to build an 8x8x8 LED Cube, but very few showcase practical applications. During an exhibition, we came across the Grove Vision AI V2 module, which inspired us to combine the two into a unique project.

Here’s how our system works: the camera on the Grove Vision AI V2 first recognizes a hand gesture (rock, paper, or scissors). It then sends the result via I²C to the Raspberry Pi Pico 2. The Pico 2 processes the data and communicates with the LED Cube’s PCB through the SPI interface. The PCB, equipped with eight shift registers, controls the LED Cube’s X–Y plane, while an additional perfboard manages the Z-axis. We will present the hardware setup in the next section.

This project is significant for two main reasons. First, we developed a Raspberry Pi Pico 2 library for the Grove Vision AI V2, making it easier for developers to integrate and leverage its vision features. Second, we created a generalized Raspberry Pi Pico library for the 8x8x8 LED Cube, allowing developers to easily modify display patterns simply by adjusting the code.

## Getting Started
