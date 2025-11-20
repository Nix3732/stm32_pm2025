#include <stdint.h>
#define __IO volatile

typedef struct {
    __IO uint32_t CRL, CRH, IDR, ODR, BSRR, BRR, LCKR;
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR, AHBENR, APB2ENR, APB1ENR, BDCR, CSR;
} RCC_TypeDef;

typedef struct {
    __IO uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, I2SCFGR, I2SPR;
} SPI_TypeDef;

#define PERIPH_BASE       0x40000000U
#define APB2PERIPH_BASE   (PERIPH_BASE + 0x10000U)
#define AHBPERIPH_BASE    (PERIPH_BASE + 0x20000U)

#define GPIOA_BASE        (APB2PERIPH_BASE + 0x0800U)
#define GPIOB_BASE        (APB2PERIPH_BASE + 0x0C00U)
#define GPIOC_BASE        (APB2PERIPH_BASE + 0x1000U)
#define RCC_BASE          (AHBPERIPH_BASE + 0x1000U)
#define SPI1_BASE         (APB2PERIPH_BASE + 0x3000U)

#define GPIOA             ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB             ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC             ((GPIO_TypeDef *)GPIOC_BASE)
#define RCC               ((RCC_TypeDef *)RCC_BASE)
#define SPI1              ((SPI_TypeDef *)SPI1_BASE)

#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64

static uint8_t display_memory[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

#define CHIP_SELECT       (1 << 0)
#define DATA_COMMAND      (1 << 1)
#define RESET_PIN         (1 << 10)

void initialize_spi_peripheral(void);
void transmit_spi_byte(uint8_t byte_value);
uint8_t receive_spi_byte(void);

static void wait_cycles(uint32_t cycle_count) {
    for (volatile uint32_t i = 0; i < cycle_count; i++);
}

static void send_display_command(uint8_t command_byte) {
    GPIOB->BRR = DATA_COMMAND;
    GPIOB->BRR = CHIP_SELECT;
    transmit_spi_byte(command_byte);
    GPIOB->BSRR = CHIP_SELECT;
}

static void send_display_data(uint8_t data_byte) {
    GPIOB->BSRR = DATA_COMMAND;
    GPIOB->BRR = CHIP_SELECT;
    transmit_spi_byte(data_byte);
    GPIOB->BSRR = CHIP_SELECT;
}

void SSD1306_Init(void) {
    RCC->APB2ENR |= (1 << 3);
    GPIOB->CRL &= ~(0xF << 0);
    GPIOB->CRL |= (0x3 << 0);
    GPIOB->CRL &= ~(0xF << 4);
    GPIOB->CRL |= (0x3 << 4);
    GPIOB->CRH &= ~(0xF << 8);
    GPIOB->CRH |= (0x3 << 8);
    
    initialize_spi_peripheral();
    
    GPIOB->BRR = RESET_PIN;
    wait_cycles(10000);
    GPIOB->BSRR = RESET_PIN;
    wait_cycles(10000);
    
    send_display_command(0xAE);
    send_display_command(0x20);
    send_display_command(0x00);
    send_display_command(0x21);
    send_display_command(0x00);
    send_display_command(0x7F);
    send_display_command(0x22);
    send_display_command(0x00);
    send_display_command(0x07);
    send_display_command(0x40);
    send_display_command(0xA1);
    send_display_command(0xC8);
    send_display_command(0xDA);
    send_display_command(0x12);
    send_display_command(0x81);
    send_display_command(0x7F);
    send_display_command(0xA4);
    send_display_command(0xA6);
    send_display_command(0xD5);
    send_display_command(0x80);
    send_display_command(0x8D);
    send_display_command(0x14);
    send_display_command(0xAF);
    
    for (unsigned int i = 0; i < sizeof(display_memory); i++) {
        display_memory[i] = 0x00;
    }
    
    for (uint8_t page_num = 0; page_num < 8; page_num++) {
        send_display_command(0xB0 + page_num);
        send_display_command(0x00);
        send_display_command(0x10);
        
        for (uint8_t column = 0; column < SSD1306_WIDTH; column++) {
            send_display_data(display_memory[page_num * SSD1306_WIDTH + column]);
        }
    }
}

void SSD1306_Clear(void) {
    for (unsigned int i = 0; i < sizeof(display_memory); i++) {
        display_memory[i] = 0x00;
    }
}

void SSD1306_Update(void) {
    for (uint8_t page_num = 0; page_num < 8; page_num++) {
        send_display_command(0xB0 + page_num);
        send_display_command(0x00);
        send_display_command(0x10);
        
        for (uint8_t column = 0; column < SSD1306_WIDTH; column++) {
            send_display_data(display_memory[page_num * SSD1306_WIDTH + column]);
        }
    }
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    uint16_t memory_index = x + (y / 8) * SSD1306_WIDTH;
    
    if (color) {
        display_memory[memory_index] |= (1 << (y % 8));
    } else {
        display_memory[memory_index] &= ~(1 << (y % 8));
    }
}

void SSD1306_DrawChessBoard(void) {
    const uint8_t cell_size = 8;
    
    for (uint8_t y_pos = 0; y_pos < SSD1306_HEIGHT; y_pos++) {
        for (uint8_t x_pos = 0; x_pos < SSD1306_WIDTH; x_pos++) {
            uint8_t cell_x = x_pos / cell_size;
            uint8_t cell_y = y_pos / cell_size;
            
            if ((cell_x + cell_y) % 2 == 0) {
                SSD1306_DrawPixel(x_pos, y_pos, 1);
            } else {
                SSD1306_DrawPixel(x_pos, y_pos, 0);
            }
        }
    }
}

void initialize_spi_peripheral(void) {
    // SPI initialization code would be here
}

void transmit_spi_byte(uint8_t byte_value) {
    // SPI transmit implementation
}

uint8_t receive_spi_byte(void) {
    // SPI receive implementation
    return 0;
}
