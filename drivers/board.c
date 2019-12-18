/*
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-12     xieming      the first commit
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"

//Configures Vector Table base location.
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}


//Configures the SysTick for OS tick
void  SysTick_Configuration(void)
{
	RCC_ClocksTypeDef  rcc_clocks;

	RCC_GetClocksFreq(&rcc_clocks);

	//SysTick_Config(rcc_clocks.HCLK_Frequency / RT_TICK_PER_SECOND);
	
	SysTick_Config(rcc_clocks.SYSCLK_Frequency / RT_TICK_PER_SECOND);
	//SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}


//This is the timer interrupt service routine.
void SysTick_Handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}

//This function will initial STM32 board
void rt_hw_board_init()
{
	/* NVIC Configuration */
	NVIC_Configuration();

	/* Configure the SysTick */
	SysTick_Configuration();

#ifdef USE_DRV_USART
	rt_hw_usart_init();
#endif

#ifdef USE_DRV_GPIO_LED
	rt_hw_led_init();
#endif

	
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
	
#if defined(RT_USING_CONSOLE) && defined(USE_DRV_USART)
	rt_console_set_device(CONSOLE_DEVICE);
#endif

}
