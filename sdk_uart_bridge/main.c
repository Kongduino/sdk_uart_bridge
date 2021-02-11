/*
   Copyright (c) 2020 Bouffalolab.

   This file is part of
 *     *** Bouffalolab Software Dev Kit ***
        (see www.bouffalolab.com).

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:
     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
     3. Neither the name of Bouffalo Lab nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <FreeRTOS.h>
#include <stdio.h>
#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <event_device.h>
#include <bl_uart.h>
#include <bl_chip.h>
#include <bl_timer.h>
#include <hal_uart.h>
#include <hal_sys.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <blog.h>
#include <bl_sys_time.h>
#include <main_board.h>
#include <string.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <device/vfs_uart.h>
#include <bl602_uart.h>
#include <utils_log.h>
#include <fdt.h>
#include <libfdt.h>
#include "demo.h"

void vAssertCalled(void) {
  volatile uint32_t ulSetTo1ToExitFunction = 0;
  taskDISABLE_INTERRUPTS();
  while (ulSetTo1ToExitFunction != 1) {
    __asm volatile( "NOP" );
  }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName ) {
  puts("Stack Overflow checked\r\n");
  while (1) {
    /*empty here*/
  }
}

void vApplicationMallocFailedHook(void) {
  printf("Memory Allocate Failed. Current left size is %d bytes\r\n", xPortGetFreeHeapSize());
  while (1) {
    /*empty here*/
  }
}

void vApplicationIdleHook(void) {
  __asm volatile(
    "   wfi     "
  );
  /*empty*/
}

void log_step(const char *step[2]) {
  printf("%s   %s\r\n", step[0], step[1]);
}

void user_vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName ) {
  /*empty*/
}

void user_vApplicationMallocFailedHook(void) {
  printf("Memory Allocate Failed. Current left size is %d bytes\r\n", xPortGetFreeHeapSize());
  /*empty*/
}

void user_vApplicationIdleHook(void) {
  __asm volatile(
    "   wfi     "
  );
  /*empty*/
}

static void _dump_boot_info(void) {
  char chip_feature[40];
  const char *banner;
  puts("Booting BL602 Chip...\r\n");
  /*Display Banner*/
  if (0 == bl_chip_banner(&banner)) {
    puts(banner);
  }
  puts("\r\n");
  /*Chip Feature list*/
  puts("\r\n");
  puts("------------------------------------------------------------\r\n");
  puts("RISC-V Core Feature:");
  bl_chip_info(chip_feature);
  puts(chip_feature);
  puts("\r\n");
  puts("Build Version: ");
  puts(BL_SDK_VER); // @suppress("Symbol is not resolved")
  puts("\r\n");
  puts("Build Date: ");
  puts(__DATE__);
  puts("\r\n");
  puts("Build Time: ");
  puts(__TIME__);
  puts("\r\n");
  puts("------------------------------------------------------------\r\n");
}

// Let's be an asshole and do it the Arduino way!
int fd1;
uint16_t idx = 0;
uint8_t buf_recv[256];

void handleString() {
  printf("%s\n", buf_recv);
  if (memcmp(buf_recv, "quit!", 5) == 0) {
    puts("You a quitter now?");
  }
}

void setup() {
  uint32_t t0 = bl_timer_now_us() / 1000;
  uint32_t t1 = bl_timer_now_us() / 1000;
  while(t1 - t0 < 2000) t1 = bl_timer_now_us() / 1000;
  /*
     Init UART0 using pins 16+7 (TX+RX) and baudrate of 2,000,000
     Init UART1 using pins  4+3 (TX+RX) and baudrate of 115,200
  */
  bl_uart_init(0, 16, 7, 255, 255, 2000000);
  puts("\n\n\nttyS0 inited\n");
  bl_uart_init(1, 4, 3, 255, 255, 115200);
  puts("ttyS1 inited\n");
  _dump_boot_info();
  puts("\n\n\nOh hai!\n\n");
  idx = 0;
  memset(buf_recv, 0, 256);
}

void loop() {
  // from int32_t hal_uart_recv_II() in hal_uart.c
  int ch;
  while (idx < 255 && (ch = bl_uart_data_recv(1)) >= 0) {
    buf_recv[idx++] = (uint8_t) ch;
  }
  printf("%s\n", buf_recv);
}

void bfl_main(void) {
  setup();
  vTaskDelay(100);
  while (true) {
    loop();
    vTaskDelay(10);
  }
}
