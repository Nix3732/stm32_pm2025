#include <stdint.h>
#include <stm32f10x.h>

typedef struct {
    uint32_t delay;
    int index;
} freq_t;

const freq_t freqs[] = {
    {64000000, -6},
    {32000000, -5},
    {16000000, -4},
    {8000000,  -3},
    {4000000,  -2},
    {2000000,  -1},
    {1000000,   0},
    {500000,    1},
    {250000,    2},
    {125000,    3},
    {62500,     4},
    {31250,     5},
    {15625,     6}
};

const int TOTAL_FREQS = 13;
int now_freq_idx = 6;
int freq_updated = 0;

void wait(uint32_t count) {
    volatile uint32_t i;
    for (i = 0; i < count; i++) {
        __NOP();
    }
}

void setup_led(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~GPIO_CRH_CNF13;
    GPIOC->CRH |= GPIO_CRH_MODE13_0;
}

void setup_btns(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1);
    GPIOB->CRL |= (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1);
    GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1;
}

void check_btns(void) {
    if (!(GPIOB->IDR & GPIO_IDR_IDR0)) {
        if (now_freq_idx < TOTAL_FREQS - 1) {
            now_freq_idx++;
            freq_updated = 1;
        }
        while (!(GPIOB->IDR & GPIO_IDR_IDR0)) {
            wait(10000);
        }
        return;
    }
    
    if (!(GPIOB->IDR & GPIO_IDR_IDR1)) {
        if (now_freq_idx > 0) {
            now_freq_idx--;
            freq_updated = 1;
        }
        while (!(GPIOB->IDR & GPIO_IDR_IDR1)) {
            wait(10000);
        }
        return;
    }
}

int main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    setup_led();
    setup_btns();
    
    while (1) {
        check_btns();
        if (freq_updated) {
            for (int i = 0; i < 3; i++) {
                GPIOC->ODR |= GPIO_ODR_ODR13;
                wait(100000);
                GPIOC->ODR &= ~GPIO_ODR_ODR13;
                wait(100000);
            }
            freq_updated = 0;
        }
        
        GPIOC->ODR |= GPIO_ODR_ODR13;
        wait(freqs[now_freq_idx].delay / 2);
        
        GPIOC->ODR &= ~GPIO_ODR_ODR13;
        wait(freqs[now_freq_idx].delay / 2);
    }
}
