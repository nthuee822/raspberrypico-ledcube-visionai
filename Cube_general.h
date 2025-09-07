#ifndef CUBE_GENERAL_H
#define CUBE_GENERAL_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_TX 19       // SPI MOSI
#define PIN_SCK 18      // SPI CLK
#define PIN_RCLK 16     // Latch pin for 74HC595

// GPIO Defines
// These pins are used to controll the eight MOSFETs, which controll which layer should light up
// Pin numbers are 6~13, from the bottom to the top
#define PIN_BASED_LAYER 6

// Converse of the High and Low
#define DATA_HIGH 0
#define DATA_LOW 1
#define LAYER_HIGH 1
#define LAYER_LOW 0

extern uint8_t temp[8][8];
extern uint8_t layer[8][8];
extern uint8_t rock[8][8];
extern uint8_t paper[8][8];
extern uint8_t scissors[8][8];

extern repeating_timer_t timer_moving_layer;
extern repeating_timer_t timer_rock;
extern repeating_timer_t timer_paper;
extern repeating_timer_t timer_scissors;

#ifdef __cplusplus
extern "C" {
#endif
void cube_init();
void input_all(uint8_t cube[8][8]);
void moving_layer();
bool timer_callback_moving_layer(repeating_timer_t *rt);
bool timer_callback_rock(repeating_timer_t *rt);
bool timer_callback_paper(repeating_timer_t *rt);
bool timer_callback_scissors(repeating_timer_t *rt);

#ifdef __cplusplus
}
#endif

#endif