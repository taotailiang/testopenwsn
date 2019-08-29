/**
\brief openmoteSTM32 definition of the "leds" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
*/
//#include "stm32f10x_lib.h"//zyx
#include "stm32l4xx_hal.h"
#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void Delay(void);

//=========================== public ==========================================

void leds_init() {
#if 0//zyx
    // Enable GPIOC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
}

// red
void leds_error_on() {
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
    //GPIOC->ODR |= 0X0040;//zyx
}

void leds_error_off() {
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
    //GPIOC->ODR &= ~0X0040;//zyx
}

void leds_error_toggle() {
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
   //GPIOC->ODR ^= 0X0040;//zyx
}

uint8_t leds_error_isOn(){
#if 0
    u8 bitstatus = 0x00;
    if ((GPIOC->ODR & 0X0040) != (u32)0) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
#endif
	return 0;
}

void leds_error_blink(){
    
    for(int i=0;i<16;i++) {   //3S almost  lcg 20180530  ;20180613 make sure 2.866Sec.
        leds_error_toggle();
        Delay();
    }
  
}

// green
void leds_radio_on() {
    //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
    //GPIOC->ODR |= 0X0080;//zyx
}

void leds_radio_off() {
    //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
    //GPIOC->ODR &= ~0X0080;//zyx
}

void leds_radio_toggle() {
    //HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
    //GPIOC->ODR ^= 0X0080;//zyx
}

uint8_t leds_radio_isOn() {
    #if 0//zyx
    u8 bitstatus = 0x00;
    if ((GPIOC->ODR & 0X0080) != (u32)0) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
    #endif
    return 0;
}

// blue
void leds_sync_on() {
    
    //GPIOC->ODR |= 0X0400;//zyx
}

void leds_sync_off() {
    
    //GPIOC->ODR &= ~0X0400;//zyx
}

void leds_sync_toggle() {
    
    //GPIOC->ODR ^= 0X0400;//zyx
}

uint8_t leds_sync_isOn() {
    #if 0
    u8 bitstatus = 0x00;
    if ((GPIOC->ODR & 0X0400) != (u32)0){
        bitstatus = 0x01;
    } else{
        bitstatus = 0x00;
    }
    return bitstatus;
    #endif
    return 0;
}

// yellow
void leds_debug_on() {
    
    //GPIOC->ODR |= 0X0800;//zyx  
}

void leds_debug_off(){
    
    //GPIOC->ODR &= ~0X0800;//zyx  
}

void leds_debug_toggle(){
    
    //GPIOC->ODR ^= 0X0800;//zyx  
}

uint8_t leds_debug_isOn(){
    #if 0//zyx
    u8 bitstatus = 0x00;
    if ((GPIOC->ODR & 0X0800) != (u32)0){
        bitstatus = 0x01;
    } else{
        bitstatus = 0x00;
    }
    return bitstatus;
    #endif
    return 0;
}

void leds_all_on() {
    
    //GPIOC->ODR |= 0X0CC0;//zyx
}

void leds_all_off() {
    //GPIOC->ODR &= ~0X0CC0;//zyx
}
void leds_all_toggle() {
    
    //GPIOC->ODR ^= 0X0CC0;//zyx
}

void leds_circular_shift() {

    #if 0//zyx
    GPIOC->ODR ^= 0X0040;
    Delay();
    GPIOC->ODR ^= 0X0040;
    Delay();
    GPIOC->ODR ^= 0X0080;
    Delay();
    GPIOC->ODR ^= 0X0080;
    Delay();
    GPIOC->ODR ^= 0X0400;
    Delay();
    GPIOC->ODR ^= 0X0400;
    Delay();
    GPIOC->ODR ^= 0X0800;
    Delay();
    GPIOC->ODR ^= 0X0800;
    Delay();
    #endif
}

void leds_increment() {
    
}

//=========================== private =========================================

void Delay(void){
    
    unsigned long ik;
    for(ik=0;ik<0x7fff8;ik++) ;
}
