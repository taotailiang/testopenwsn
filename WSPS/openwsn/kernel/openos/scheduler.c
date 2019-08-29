/**
\brief OpenOS scheduler.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#include "opendefs.h"
#include "scheduler.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "stm32l4xx_hal.h"
#include "zyx.h"
#include "S2LP_Config.h"
#include "uart.h"
uint8_t txbuf[20]={1,2,3,4,5,6,7,8,9,10};
uint8_t rxbuf[10];
HAL_StatusTypeDef testzyx;

extern SPI_HandleTypeDef hspi2;
//=========================== variables =======================================

scheduler_vars_t scheduler_vars;
scheduler_dbg_t  scheduler_dbg;

//=========================== prototypes ======================================

void consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init() {   
   
   // initialization module variables
   memset(&scheduler_vars,0,sizeof(scheduler_vars_t));
   memset(&scheduler_dbg,0,sizeof(scheduler_dbg_t));
   
   // enable the scheduler's interrupt so SW can wake up the scheduler
   SCHEDULER_ENABLE_INTERRUPT();
}

void scheduler_start() {
   taskList_item_t* pThisTask;
   #if 0
	while(1)
	{
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFE);
	}
#endif
    while (1) 
   {
   		
      while(scheduler_vars.task_list!=NULL) 
      {
         // there is still at least one task in the linked-list of tasks
         //HAL_GPIO_WritePin(GPIO_LED1,GPIO_PIN_LED1,GPIO_PIN_SET);
    	 INTERRUPT_DECLARATION();
    	 DISABLE_INTERRUPTS();
    	 debugpins_task_set(); // lcg 20180411
         // the task to execute is the one at the head of the queue
         pThisTask                = scheduler_vars.task_list;
         
         // shift the queue by one task
         scheduler_vars.task_list = pThisTask->next;
         
         ENABLE_INTERRUPTS();

         // execute the current task
         pThisTask->cb();
         
         // free up this task container
         pThisTask->cb            = NULL;
         pThisTask->prio          = TASKPRIO_NONE;
         pThisTask->next          = NULL;
         scheduler_dbg.numTasksCur--;
         debugpins_task_clr(); // lcg 20180411
      }
      //debugpins_task_clr();
      if(!UARTDMA_IS_BUSY())
      {
      		board_sleep();
      }
      //debugpins_task_set();                      // IAR should halt here if nothing to do
      //S2LPSpiWriteFifo(20,txbuf);
	  //S2LPCmdStrobeTx();
      //HAL_GPIO_WritePin(GPIO_RFSDN,GPIO_PIN_RFSDN,GPIO_PIN_RESET);
   }
}

 void scheduler_push_task(task_cbt cb, task_prio_t prio) {
   taskList_item_t*  taskContainer;
   taskList_item_t** taskListWalker;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   
   // find an empty task container
   taskContainer = &scheduler_vars.taskBuf[0];
   while (taskContainer->cb!=NULL &&
          taskContainer<=&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) 
   {
      taskContainer++;
   }
   if (taskContainer>&scheduler_vars.taskBuf[TASK_LIST_DEPTH-1]) {
      // task list has overflown. This should never happpen!
   
      // we can not print from within the kernel. Instead:
      // blink the error LED
      leds_error_blink();  //lcg 20180613 ,the day 20180611 dead here at last.
      // reset the board
      board_reset();
   }
   // fill that task container with this task
   taskContainer->cb              = cb;
   taskContainer->prio            = prio;
   
   // find position in queue
   taskListWalker                 = &scheduler_vars.task_list;
   while (*taskListWalker!=NULL &&
          (*taskListWalker)->prio <= taskContainer->prio) {
      taskListWalker              = (taskList_item_t**)&((*taskListWalker)->next);
   }
   // insert at that position
   taskContainer->next            = *taskListWalker;
   *taskListWalker                = taskContainer;
   // maintain debug stats
   scheduler_dbg.numTasksCur++;
   if (scheduler_dbg.numTasksCur>scheduler_dbg.numTasksMax) {
      scheduler_dbg.numTasksMax   = scheduler_dbg.numTasksCur;
   }
   
   ENABLE_INTERRUPTS();
}

//=========================== private =========================================
