/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include <string.h>

#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/interrupt.h>
#include <driverlib/prcm.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

extern int v7_example(void);
extern void (*const g_pfnVectors[])(void);

int main(void) {
  MAP_IntVTableBaseSet((unsigned long) &g_pfnVectors[0]);
  MAP_IntEnable(FAULT_SYSTICK);
  MAP_IntMasterEnable();
  PRCMCC3200MCUInit();
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IOLBF, 0);

  return v7_example();
}
