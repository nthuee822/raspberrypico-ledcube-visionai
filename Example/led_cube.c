#include <stdio.h>
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

void clear_cube();
void z_layer(int z);
void all_layer();
void all_light_up();
void moving_forward_plane();
void moving_backward_plane();
void moving_forward_plane2();
void moving_backward_plane2();
void ripple();
void drip();
void upward();
void downward();

int main()
{
    
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 100000);
    spi_set_format(SPI_PORT, 8, 1, 0, SPI_MSB_FIRST);
    gpio_set_function(PIN_TX, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);

    // GPIO initialization.
    gpio_init(PIN_RCLK);
    gpio_set_dir(PIN_RCLK, GPIO_OUT);
    gpio_put(PIN_RCLK, DATA_LOW);       // set to low

    for (int i = 0; i < 8; i++) {
        gpio_init(PIN_BASED_LAYER + i);
        gpio_set_dir(PIN_BASED_LAYER + i, GPIO_OUT);
        gpio_put(PIN_BASED_LAYER + i, 0);      // set to low
    }
    
    clear_cube();

    for (int i = 0; i < 300; i++) {
        all_light_up();
    }
    while (true) {
        upward();
        downward();
        moving_forward_plane();
        moving_backward_plane();
        moving_forward_plane2();
        moving_backward_plane2();

        for (int i = 0; i < 5; i++) {
            drip();
            ripple();
        }
    }
}

void clear_cube() {
    uint8_t layer[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    spi_write_blocking(SPI_PORT, layer, 8);

    gpio_put(PIN_RCLK, 1);
    sleep_us(1);
    gpio_put(PIN_RCLK, 0);
}


void all_layer() {
    for (int z = 0; z < 8; z++) {
        gpio_put(PIN_BASED_LAYER + z, 1);
        sleep_ms(1);
        gpio_put(PIN_BASED_LAYER + z, 0);
    }
}

void z_layer(int z) {
    int i;

    for (i = 0; i < 8; i++) {
        gpio_put(PIN_BASED_LAYER + i, i == z ? 1 : 0);
    }
}

void all_light_up() {

    uint8_t layer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    spi_write_blocking(SPI_PORT, layer, 8);

    gpio_put(PIN_RCLK, 1);
    sleep_us(1);
    gpio_put(PIN_RCLK, 0);
    
    all_layer();
}

void moving_forward_plane() {
    uint8_t layer[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    for (int i = 0; i < 8; i++) {
        spi_write_blocking(SPI_PORT, layer, 8);

        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);

        for(int j = 0; j < 10; j++) {
            all_layer();
        }
        sleep_ms(10);
        
        layer[i + 1] = layer[i];
    }
}

void moving_backward_plane() {
    uint8_t layer[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    
    for (int i = 7; i >= 0; i--) {
        spi_write_blocking(SPI_PORT, layer, 8);

        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);

        for(int j = 0; j < 10; j++) {
            all_layer();
        }
        sleep_ms(10);
        
        layer[i - 1] = layer[i];
    }
}

void moving_forward_plane2() {
    uint8_t layer[8] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
    
    for (int i = 0; i < 8; i++) {
        spi_write_blocking(SPI_PORT, layer, 8);

        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);

        for (int j = 0; j < 10; j++) {
            all_layer();
        }
        sleep_ms(20);
        
        for (int j = 0; j < 8; j++) {
            layer[j] = (layer[j] << 1);
        }
    }
}

void moving_backward_plane2() {
    uint8_t layer[8] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
    
    for (int i = 0; i < 8; i++) {
        spi_write_blocking(SPI_PORT, layer, 8);

        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);

        for (int j = 0; j < 10; j++) {
            all_layer();
        }
        sleep_ms(20);
        
        for (int j = 7; j >= 0; j--) {
            layer[j] = (layer[j] >> 1);
        }
    }
}

void ripple() {
    uint8_t circle[7][8] = {{0xFF, 0xFF, 0xFF, 0xE7, 0xE7, 0xFF, 0xFF, 0xFF},
                            {0xFF, 0xFF, 0xE7, 0xDB, 0xDB, 0xE7, 0xFF, 0xFF},
                            {0xFF, 0xE7, 0xDB, 0xBD, 0xBD, 0xDB, 0xE7, 0xFF},
                            {0xE7, 0xDB, 0xBD, 0x7E, 0x7E, 0xBD, 0xDB, 0xE7},
                            {0xDB, 0xBD, 0x7E, 0xFF, 0xFF, 0x7E, 0xBD, 0xDB},
                            {0xBD, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0xBD},
                            {0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E},      // circle of different radius
                           };
    
    for (int i = 0; i < 7; i++) {
        spi_write_blocking(SPI_PORT, circle[i], 8);
        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);

        z_layer(0);
        sleep_ms(100);
    }

}

void drip() {
    uint8_t drip[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF};
    
    spi_write_blocking(SPI_PORT, drip, 8);
    gpio_put(PIN_RCLK, 1);
    sleep_us(1);
    gpio_put(PIN_RCLK, 0);

    for (int i = 7; i > 0; i--) {
        z_layer(i);
        sleep_ms(100);
    }

                           
}

void upward() {
    uint8_t plane[8][8] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},       // circle of different radius
                           };
    for (int j = 0; j < 8; j++) {
        spi_write_blocking(SPI_PORT, plane[j], 8);
        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);
          
        z_layer(j);
        sleep_ms(200);
    }
}

void downward() {
    uint8_t plane[8][8] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},       // circle of different radius
                           };
    for (int j = 0; j < 8; j++) {
        spi_write_blocking(SPI_PORT, plane[j], 8);
        gpio_put(PIN_RCLK, 1);
        sleep_us(1);
        gpio_put(PIN_RCLK, 0);
          
        z_layer(7 - j);
        sleep_ms(200);
    }
}