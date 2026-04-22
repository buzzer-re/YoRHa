#pragma once

#ifndef FIRMWARE
#define FIRMWARE 900
#endif
#if FIRMWARE == 700
  #include "fw700.h"
#elif FIRMWARE == 750
  #include "fw750.h"
#elif FIRMWARE == 800
  #include "fw800.h"
#elif FIRMWARE == 850
  #include "fw850.h"
#elif   FIRMWARE == 900
  #include "fw900.h"
#elif FIRMWARE == 903
  #include "fw903.h"
#endif