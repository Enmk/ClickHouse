#include <common/TimeZone.h>

#include <string>

TimeZone::TimeZone(const std::string & timezone_name)
    : DateLUTImpl(timezone_name),
      extended_lut(*this)
{
}

TimeZone::~TimeZone()
{}
