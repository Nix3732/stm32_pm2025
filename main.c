#include <stdint.h>
#include <stm32f10x.h>

typedef struct {
    uint32_t pause_duration;
    int scale_factor;
} blink_config_t;

static const blink_config_t blink_speeds[] = {
    {64000000, -6}, {32000000, -5}, {16000000, -4},
    {8000000,  -3}, {4000000,  -2}, {2000000,  -1},
    {1000000,   0}, {500000,    1}, {250000,    2},
    {125000,    3}, {62500,     4}, {31250,     5},
    {15625,     6}
};

#define SPEED_COUNT (sizeof(blink_speeds) / sizeof(blink_speeds[0]))
#define DEFAULT_SPEED_INDEX 6

static uint8_t active_speed_index = DEFAULT_SPEED_INDEX;
static uint8_t speed_updated_flag = 0;

static void wait_cycles(uint32_t cycle_count) {
    volatile uint32_t counter;
    for (counter = 0; counter < cycle_count; counter++) {
        __asm__("nop");
    }
}

static void setup_led_gpio(void) {
    RCC->APB2ENR |= (1 << 4);
    GPIOC->CRH = (GPIOC->CRH & ~(0xF << 20)) | (0x2 << 20);
}

static void setup_button_gpio(void) {
    RCC->APB2ENR |= (1 << 3);
    GPIOB->CRL = (GPIOB->CRL & ~(0xF << 0)) | (0x8 << 0);
    GPIOB->CRL = (GPIOB->CRL & ~(0xF << 4)) | (0x8 << 4);
    GPIOB->ODR |= (1 << 0) | (1 << 1);
}

static void process_button_input(void) {
    static uint8_t prev_button0 = 1, prev_button1 = 1;
    uint8_t current_button0, current_button1;
    
    current_button0 = (GPIOB->IDR & (1 << 0)) ? 1 : 0;
    current_button1 = (GPIOB->IDR & (1 << 1)) ? 1 : 0;
    
    if (current_button0 == 0 && prev_button0 == 1) {
        if (active_speed_index < SPEED_COUNT - 1) {
            active_speed_index++;
            speed_updated_flag = 1;
        }
        while ((GPIOB->IDR & (1 << 0)) == 0) {
            wait_cycles(5000);
        }
    }
    
    if (current_button1 == 0 && prev_button1 == 1) {
        if (active_speed_index > 0) {
            active_speed_index--;
            speed_updated_flag = 1;
        }
        while ((GPIOB->IDR & (1 << 1)) == 0) {
            wait_cycles(5000);
        }
    }
    
    prev_button0 = current_button0;
    prev_button1 = current_button1;
}

static void show_speed_change(void) {
    uint8_t i;
    
    for (i = 0; i < 3; i++) {
        GPIOC->BSRR = (1 << 13);
        wait_cycles(80000);
        GPIOC->BRR = (1 << 13);
        wait_cycles(80000);
    }
    
    speed_updated_flag = 0;
}

int main(void) {
    uint32_t blink_interval;
    
    RCC->APB2ENR |= (1 << 0);
    
    setup_led_gpio();
    setup_button_gpio();
    
    while (1) {
        process_button_input();
        
        if (speed_updated_flag) {
            show_speed_change();
        }
        
        blink_interval = blink_speeds[active_speed_index].pause_duration / 2;
        
        GPIOC->BSRR = (1 << 13);
        wait_cycles(blink_interval);
        
        GPIOC->BRR = (1 << 13);
        wait_cycles(blink_interval);
    }
    
    return 0;
}
