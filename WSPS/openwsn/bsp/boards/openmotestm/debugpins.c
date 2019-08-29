/**
\brief opemnstm32 definition of the "debugpins" bsp module.

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, February 2012.
*/
//#include "stm32f10x_lib.h"//zyx
#include "stm32l4xx_hal.h"
#include "debugpins.h"
#include  "zyx.h"
//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() 
{

}

// PC.5
void debugpins_frame_toggle(){
	//////HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_2);
    //GPIOC->ODR ^= 0X0020;//zyx
}
void debugpins_frame_clr() {
    
    //GPIOC->ODR &= ~0X0020;//zyx
}
void debugpins_frame_set() {
	
    //HAL_GPIO_WritePin(GPIO_LED3,GPIO_PIN_LED3,GPIO_PIN_SET);
    //GPIOC->ODR |=  0X0020;//zyx
}   

// PA.7
void debugpins_slot_toggle() {

	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_9);

}
void debugpins_slot_clr() {
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
    //GPIOA->ODR &= ~0X0080;//zyx
}
void debugpins_slot_set() {
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
    //GPIOA->ODR |=  0X0080;//zyx
}

// PA.5
void debugpins_fsm_toggle() {
	///HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_3);
}
void debugpins_fsm_clr() {
	///HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
}
void debugpins_fsm_set() {
	///HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
}

// PA.6
void debugpins_task_toggle() 
{
    
    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
}
void debugpins_task_clr() 
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
}
void debugpins_task_set() 
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
}

// PC.1
void debugpins_isr_toggle() {
	HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_10);
	//HAL_GPIO_TogglePin(GPIO_LED0,GPIO_PIN_LED0);
   // HAL_GPIO_TogglePin(GPIO_LED1,GPIO_PIN_LED1);
    //GPIOC->ODR ^=  0X0002;//zyx
    //HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
}
void debugpins_isr_clr() {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET);
	// HAL_GPIO_WritePin(GPIO_LED0,GPIO_PIN_LED0,GPIO_PIN_RESET);
   // HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1, GPIO_PIN_RESET);
    //GPIOC->ODR &= ~0X0002;//zyx
    //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
}
void debugpins_isr_set() {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
	 //HAL_GPIO_WritePin(GPIO_LED0,GPIO_PIN_LED0,GPIO_PIN_SET);
   // HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1, GPIO_PIN_SET);
    //GPIOC->ODR |= 0X0002;//zyx
    //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
}

// PC.0
void debugpins_radio_toggle() {
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_4);
}
void debugpins_radio_clr() {

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);

}
void debugpins_radio_set() {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);

}



