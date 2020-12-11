#pragma once

#include <common/types.h>
#include <common/strong_typedef.h>

/** Represents number of days since 1970-01-01.
  * See DateLUTImpl for usage examples.
  */
STRONG_TYPEDEF(UInt16, DayNum)
STRONG_TYPEDEF(Int32, GlobalDayNum) // covers pre-epoch (1970-01-01) and post-LUT-0 (2105) dates.
