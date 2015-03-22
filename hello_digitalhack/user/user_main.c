#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "driver/uart.h"

LOCAL os_timer_t hello_timer;

LOCAL void ICACHE_FLASH_ATTR hello_cb(void *arg) {
  // Print message on UART0 pins 25 and 26 
	ets_uart_printf("Hello Digitalhack - UART0!!!\n");
  
  //Print message on UART1, tx only, on pin 20
  os_printf("Hello Digitalhack - UART1!!!\n");
}

void user_init(void) {
	// Configure the UART0 and UART1 (TX only) to 115200
	uart_init(BIT_RATE_115200,BIT_RATE_115200);

	// Reset timer in case it was used before.
	os_timer_disarm(&hello_timer);

	// Set a callback function (arg2) to be called when the time expires (arg1) 
  // passing no parameters to the callback function (arg3)
	os_timer_setfn(&hello_timer, (os_timer_func_t *)hello_cb, (void *)0);
  
	// Setup the timer (arg1) to be called in 1000ms (arg2) and to repeat (arg3).
	os_timer_arm(&hello_timer, 1000, 1);
}
