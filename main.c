#include <stdint.h>

void wait_cycles(uint32_t cycle_count) {
    for (volatile uint32_t i = 0; i < cycle_count; i++);
}

void initialize_spi_peripheral(void);
void transmit_spi_byte(uint8_t byte_value);
uint8_t receive_spi_byte(void);
void SSD1306_Init(void);
void SSD1306_DrawChessBoard(void);
void SSD1306_Update(void);

int main(void) {
    *(volatile uint32_t*)(0x40021000 + 0x18) |= (1 << 4);
    
    *(volatile uint32_t*)(0x40011000 + 0x04) |= (1 << 20);
    
    for(int count = 0; count < 3; count++) {
        *(volatile uint32_t*)(0x40011000 + 0x0C) |= (1 << 13);
        wait_cycles(100000);
        *(volatile uint32_t*)(0x40011000 + 0x0C) &= ~(1 << 13);
        wait_cycles(100000);
    }
    
    initialize_spi_peripheral();
    SSD1306_Init();
    SSD1306_DrawChessBoard();
    SSD1306_Update();
    
    while(1) {
        *(volatile uint32_t*)(0x40011000 + 0x0C) ^= (1 << 13);
        wait_cycles(500000);
    }
}
