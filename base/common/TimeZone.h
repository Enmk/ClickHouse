#pragma once

#include "DateLUTImpl.h"
#include "DayNum.h"
#include "ExtendedDateLUTImpl.h"

#include <string>

class TimeZone : public DateLUTImpl
{
public:
    explicit TimeZone(const std::string & timezone_name);
    ~TimeZone();

    TimeZone(const TimeZone&) = delete;
    TimeZone operator=(const TimeZone&) = delete;
    TimeZone(TimeZone&&) = delete;
    TimeZone operator=(const TimeZone&&) = delete;

    const ExtendedDateLUTImpl & extendedRange() const
    {
        return extended_lut;
    }

private:
    ExtendedDateLUTImpl extended_lut;
};


