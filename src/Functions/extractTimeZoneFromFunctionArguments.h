#pragma once

#include <string>
#include <Core/ColumnNumbers.h>
#include <Core/ColumnsWithTypeAndName.h>

//#include <common/TimeZone.h>

//#include <type_traits>


class TimeZone;

namespace DB
{

class Block;
class DataTypeDateTime64;

/// Determine working timezone either from optional argument with time zone name or from time zone in DateTime type of argument.
std::string extractTimeZoneNameFromFunctionArguments(
    const ColumnsWithTypeAndName & arguments, size_t time_zone_arg_num, size_t datetime_arg_num);

const TimeZone & extractTimeZoneFromFunctionArguments(
    const ColumnsWithTypeAndName & arguments, size_t time_zone_arg_num, size_t datetime_arg_num);

//template <typename DateTimeType>
//inline const auto & extractTypeDependentTimeZoneFromFunctionArguments(Block & block, const ColumnNumbers & arguments, size_t time_zone_arg_num, size_t datetime_arg_num)
//{
//    const auto & timezone = extractTimeZoneFromFunctionArguments(block, arguments, time_zone_arg_num, datetime_arg_num);
//    if constexpr (std::is_same_v<DateTimeType, DataTypeDateTime64>)
//    {
//        return timezone;
//    }
//    else
//    {
//        return timezone.getDefaultLUT();
//    }
//}
}
