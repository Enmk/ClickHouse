#include <common/DateLUT.h>
#include <common/DateLUTImpl.h>
#include <common/TimeZone.h>

#include <gtest/gtest.h>

#include <string>

/// For the expansion of gtest macros.
#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

// how many seconds (roughly) are covered by LUT.
#define DATE_LUT_SIZE_IN_SECONDS DATE_LUT_MAX_DAY_NUM * 3600ULL * 24ULL

bool operator==(const DayNum & dn, const GlobalDayNum gdn)
{
    return dn.toUnderType() == gdn.toUnderType();
}

bool operator==(const GlobalDayNum gdn, const DayNum & dn)
{
    return gdn.toUnderType() == dn.toUnderType();
}

namespace
{

Int64 getUTCOffset()
{
    const time_t t = 0;
    tm tm;
    if (!localtime_r(&t, &tm))
    {
        assert(false && "localtime_r failed");
    }

    return tm.tm_gmtoff;
}

time_t YYYYMMDDToTimeT(unsigned value)
{
    static const auto UTC_offset = getUTCOffset();

    struct tm tm;

    memset(&tm, 0, sizeof(tm));

    tm.tm_year = value / 10000 - 1900;
    tm.tm_mon = (value % 10000) / 100 - 1;
    tm.tm_mday = value % 100;
    tm.tm_isdst = 0;

    return mktime(&tm) + UTC_offset;
}

std::tuple<Int16 /*YYYY*/, UInt8 /*MM*/, UInt8 /*DD*/, UInt8 /*hh*/, UInt8 /*mm*/, UInt8 /*ss*/>
splitYYYYMMDDhhmmss(UInt64 value)
{
    const UInt64 year_div = 10000000000;
    const UInt64 month_div = 100000000;
    const UInt64 day_div = 1000000;
    const UInt64 hour_div = 10000;
    const UInt64 minute_div = 100;
    const UInt64 second_div = 1;

    return {
        value / year_div % 10000,
        value / month_div % 100,
        value / day_div % 100,
        value / hour_div % 100,
        value / minute_div % 100,
        value / second_div % 100
    };
}

class UniversalDayNum
{
    const Int32 value = 0;
public:
    explicit UniversalDayNum(Int32 val)
        : value(val)
    {}

    inline operator DayNum() const
    {
        return DayNum{static_cast<DayNum::UnderlyingType>(value)};
    }

    inline operator GlobalDayNum() const
    {
        return GlobalDayNum{value};
    }

//    inline Int32 toUnderType() const
//    {
//        return value;
//    }
};

template <typename ...Args>
inline auto toString(Args && ...args)
{
    return (::testing::Message() << ... << args).GetString();
}

}

class TimeZone_VS_DateLUT_Test : public ::testing::TestWithParam<std::tuple<const char* /*timzone name*/, time_t>>
{};

TEST_P(TimeZone_VS_DateLUT_Test, SameAsDateLUTImpl)
{
    const auto & [timezone_name, time_value] = GetParam();

    const DateLUTImpl lut(timezone_name);
    const TimeZoneImpl tz(timezone_name);

    const auto daynum_value = UniversalDayNum(lut.toDayNum(time_value).toUnderType());
    const auto [year, month, day_of_month, hour, minute, second] = splitYYYYMMDDhhmmss(lut.toNumYYYYMMDDhhmmss(time_value));

//    SCOPED_TRACE(toString("\ntimezone: ", timezone_name, "\ndaynum_value: ", daynum_value.toUnderType(), "\ntime_value:", time_value,
//            "\nhuman time: ", year, ".", static_cast<UInt32>(month), ".", static_cast<UInt32>(day_of_month), "T", static_cast<UInt32>(hour), ":", static_cast<UInt32>(minute), ":", static_cast<UInt32>(second)));

    EXPECT_EQ(tz.getTimeZone(), lut.getTimeZone());

    EXPECT_EQ(tz.toDate(time_value), lut.toDate(time_value));
    EXPECT_EQ(tz.toDate(daynum_value), lut.toDate(daynum_value));
    EXPECT_EQ(tz.toMonth(time_value), lut.toMonth(time_value));
    EXPECT_EQ(tz.toQuarter(time_value), lut.toQuarter(time_value));
    EXPECT_EQ(tz.toYear(time_value), lut.toYear(time_value));
    EXPECT_EQ(tz.toDayOfWeek(time_value), lut.toDayOfWeek(time_value));
    EXPECT_EQ(tz.toDayOfMonth(time_value), lut.toDayOfMonth(time_value));
    EXPECT_EQ(tz.daysInMonth(daynum_value), lut.daysInMonth(daynum_value));
    EXPECT_EQ(tz.daysInMonth(time_value), lut.daysInMonth(time_value));
    EXPECT_EQ(tz.daysInMonth(year, month), lut.daysInMonth(year, month));

    for (Int32 day_shift : {-7, -1, 0, 1, 7})
    {
//        SCOPED_TRACE(toString("day_shift: ", day_shift));

        EXPECT_EQ(tz.toDateAndShift(time_value, day_shift), lut.toDateAndShift(time_value, day_shift));
    }

    EXPECT_EQ(tz.toTime(time_value), lut.toTime(time_value));
    EXPECT_EQ(tz.toHour(time_value), lut.toHour(time_value));
    EXPECT_EQ(tz.toSecond(time_value), lut.toSecond(time_value));
    EXPECT_EQ(tz.toMinute(time_value), lut.toMinute(time_value));
    EXPECT_EQ(tz.toStartOfMinute(time_value), lut.toStartOfMinute(time_value));
    EXPECT_EQ(tz.toStartOfFiveMinute(time_value), lut.toStartOfFiveMinute(time_value));
    EXPECT_EQ(tz.toStartOfFifteenMinutes(time_value), lut.toStartOfFifteenMinutes(time_value));
    EXPECT_EQ(tz.toStartOfTenMinutes(time_value), lut.toStartOfTenMinutes(time_value));
    EXPECT_EQ(tz.toStartOfHour(time_value), lut.toStartOfHour(time_value));
    EXPECT_EQ(tz.toDayNum(time_value), lut.toDayNum(time_value));

    // explicitly constructing DateLUTImpl::LutDayNum to avoid ambiguity
//    EXPECT_EQ(tz.fromDayNum(daynum_value), lut.fromDayNum(DateLUTImpl::LutDayNum(daynum_value)));
    EXPECT_EQ(tz.fromDayNum(daynum_value), lut.fromDayNum(DayNum(daynum_value)));

    EXPECT_EQ(tz.toMonth(daynum_value), lut.toMonth(daynum_value));
    EXPECT_EQ(tz.toQuarter(daynum_value), lut.toQuarter(daynum_value));
    EXPECT_EQ(tz.toYear(daynum_value), lut.toYear(daynum_value));
    EXPECT_EQ(tz.toDayOfWeek(daynum_value), lut.toDayOfWeek(daynum_value));
    EXPECT_EQ(tz.toDayOfMonth(daynum_value), lut.toDayOfMonth(daynum_value));
    EXPECT_EQ(tz.toDayOfYear(daynum_value), lut.toDayOfYear(daynum_value));
    EXPECT_EQ(tz.toDayOfYear(time_value), lut.toDayOfYear(time_value));
    EXPECT_EQ(tz.toRelativeWeekNum(daynum_value), lut.toRelativeWeekNum(daynum_value));
    EXPECT_EQ(tz.toRelativeWeekNum(time_value), lut.toRelativeWeekNum(time_value));
    EXPECT_EQ(tz.toISOYear(daynum_value), lut.toISOYear(daynum_value));
    EXPECT_EQ(tz.toISOYear(time_value), lut.toISOYear(time_value));
    EXPECT_EQ(tz.toFirstDayNumOfISOYear(daynum_value), lut.toFirstDayNumOfISOYear(daynum_value));
    EXPECT_EQ(tz.toFirstDayNumOfISOYear(time_value), lut.toFirstDayNumOfISOYear(time_value));
    EXPECT_EQ(tz.toFirstDayOfISOYear(time_value), lut.toFirstDayOfISOYear(time_value));
    EXPECT_EQ(tz.toISOWeek(daynum_value), lut.toISOWeek(daynum_value));
    EXPECT_EQ(tz.toISOWeek(time_value), lut.toISOWeek(time_value));

    for (const auto & mode : {WeekModeFlag::MONDAY_FIRST, WeekModeFlag::YEAR, WeekModeFlag::FIRST_WEEKDAY, WeekModeFlag::NEWYEAR_DAY})
    {
        const UInt8 week_mode = static_cast<UInt8>(mode);
//        SCOPED_TRACE(toString("week_mode: ", static_cast<UInt32>(week_mode)));

        EXPECT_EQ(tz.toYearWeek(daynum_value, week_mode), lut.toYearWeek(daynum_value, week_mode));
        EXPECT_EQ(tz.toFirstDayNumOfWeek(daynum_value, week_mode), lut.toFirstDayNumOfWeek(daynum_value, week_mode));
        EXPECT_EQ(tz.check_week_mode(week_mode), lut.check_week_mode(week_mode));
    }

    for (const auto monday_first_mode : {true, false})
    {
//        SCOPED_TRACE(toString("monday_first_mode: ", monday_first_mode));

        EXPECT_EQ(tz.toYearWeekOfNewyearMode(daynum_value, monday_first_mode), lut.toYearWeekOfNewyearMode(daynum_value, monday_first_mode));
    }

    for (const auto sunday_first_day_of_week : {true, false})
    {
//        SCOPED_TRACE(toString("sunday_first_day_of_week: ", sunday_first_day_of_week));

        EXPECT_EQ(tz.calc_weekday(daynum_value, sunday_first_day_of_week), lut.calc_weekday(daynum_value, sunday_first_day_of_week));
    }

    EXPECT_EQ(tz.calc_days_in_year(year), lut.calc_days_in_year(year));
    EXPECT_EQ(tz.toRelativeMonthNum(daynum_value), lut.toRelativeMonthNum(daynum_value));
    EXPECT_EQ(tz.toRelativeMonthNum(time_value), lut.toRelativeMonthNum(time_value));
    EXPECT_EQ(tz.toRelativeQuarterNum(daynum_value), lut.toRelativeQuarterNum(daynum_value));
    EXPECT_EQ(tz.toRelativeQuarterNum(time_value), lut.toRelativeQuarterNum(time_value));
    EXPECT_EQ(tz.toRelativeHourNum(time_value), lut.toRelativeHourNum(time_value));
    EXPECT_EQ(tz.toRelativeHourNum(daynum_value), lut.toRelativeHourNum(daynum_value));
    EXPECT_EQ(tz.toRelativeMinuteNum(time_value), lut.toRelativeMinuteNum(time_value));
    EXPECT_EQ(tz.toRelativeMinuteNum(daynum_value), lut.toRelativeMinuteNum(daynum_value));
    EXPECT_EQ(tz.makeDayNum(year, month, day_of_month), lut.makeDayNum(year, month, day_of_month));
    EXPECT_EQ(tz.makeDate(year, month, day_of_month), lut.makeDate(year, month, day_of_month));
    EXPECT_EQ(tz.makeDateTime(year, month, day_of_month, hour, minute, second), lut.makeDateTime(year, month, day_of_month, hour, minute, second));
//    EXPECT_EQ(tz.getValues(daynum_value), lut.getValues(daynum_value));
//    EXPECT_EQ(tz.getValues(time_value), lut.getValues(time_value));
    EXPECT_EQ(tz.toNumYYYYMM(time_value), lut.toNumYYYYMM(time_value));
    EXPECT_EQ(tz.toNumYYYYMM(daynum_value), lut.toNumYYYYMM(daynum_value));
    EXPECT_EQ(tz.toNumYYYYMMDD(time_value), lut.toNumYYYYMMDD(time_value));
    EXPECT_EQ(tz.toNumYYYYMMDD(daynum_value), lut.toNumYYYYMMDD(daynum_value));

    const auto num_YYYYMMDD = lut.toNumYYYYMMDD(time_value);
    EXPECT_EQ(tz.YYYYMMDDToDate(num_YYYYMMDD), lut.YYYYMMDDToDate(num_YYYYMMDD));
    EXPECT_EQ(tz.YYYYMMDDToDayNum(num_YYYYMMDD), lut.YYYYMMDDToDayNum(num_YYYYMMDD));

    const auto num_YYYYMMDDhhmmss = lut.toNumYYYYMMDDhhmmss(time_value);
    EXPECT_EQ(tz.toNumYYYYMMDDhhmmss(time_value), lut.toNumYYYYMMDDhhmmss(time_value));
    EXPECT_EQ(tz.YYYYMMDDhhmmssToTime(num_YYYYMMDDhhmmss), lut.YYYYMMDDhhmmssToTime(num_YYYYMMDDhhmmss));

    EXPECT_EQ(tz.saturateDayOfMonth(year, month, day_of_month), lut.saturateDayOfMonth(year, month, day_of_month));
    EXPECT_EQ(tz.timeToString(time_value), lut.timeToString(time_value));
    EXPECT_EQ(tz.dateToString(time_value), lut.dateToString(time_value));
    EXPECT_EQ(tz.dateToString(daynum_value), lut.dateToString(daynum_value));

    // =============================================================================================
    // TODO: handle crossing the LUT border
    //
    for (Int64 delta : {-7, -1, 0, 1, 7})
    {
//        SCOPED_TRACE(toString("delta:", delta));

        EXPECT_EQ(tz.addWeeks(time_value, delta), lut.addWeeks(time_value, delta));
        EXPECT_EQ(tz.addMonths(time_value, delta), lut.addMonths(time_value, delta));
        EXPECT_EQ(tz.addMonths(daynum_value, delta), lut.addMonths(daynum_value, delta));
        EXPECT_EQ(tz.addQuarters(time_value, delta), lut.addQuarters(time_value, delta));
        EXPECT_EQ(tz.addQuarters(daynum_value, delta), lut.addQuarters(daynum_value, delta));
        EXPECT_EQ(tz.addYears(time_value, delta), lut.addYears(time_value, delta));
        EXPECT_EQ(tz.addYears(daynum_value, delta), lut.addYears(daynum_value, delta));
    }

    for (UInt64 interval : {/*0, */1, 7}) // zero interval would result a run-time floating exception, caused by division by zero.
    {
//        SCOPED_TRACE(toString("interval:", interval));

        EXPECT_EQ(tz.toStartOfYearInterval(daynum_value, interval), lut.toStartOfYearInterval(daynum_value, interval));
        EXPECT_EQ(tz.toStartOfQuarterInterval(daynum_value, interval), lut.toStartOfQuarterInterval(daynum_value, interval));
        EXPECT_EQ(tz.toStartOfMonthInterval(daynum_value, interval), lut.toStartOfMonthInterval(daynum_value, interval));
        EXPECT_EQ(tz.toStartOfWeekInterval(daynum_value, interval), lut.toStartOfWeekInterval(daynum_value, interval));
        EXPECT_EQ(tz.toStartOfDayInterval(daynum_value, interval), lut.toStartOfDayInterval(daynum_value, interval));
        EXPECT_EQ(tz.toStartOfHourInterval(time_value, interval), lut.toStartOfHourInterval(time_value, interval));
        EXPECT_EQ(tz.toStartOfMinuteInterval(time_value, interval), lut.toStartOfMinuteInterval(time_value, interval));
        EXPECT_EQ(tz.toStartOfSecondInterval(time_value, interval), lut.toStartOfSecondInterval(time_value, interval));
    }

    EXPECT_EQ(tz.toFirstDayOfWeek(time_value), lut.toFirstDayOfWeek(time_value));
    EXPECT_EQ(tz.toFirstDayNumOfWeek(daynum_value), lut.toFirstDayNumOfWeek(daynum_value));

    EXPECT_EQ(tz.toFirstDayNumOfWeek(time_value), lut.toFirstDayNumOfWeek(time_value));
    EXPECT_EQ(tz.toFirstDayOfMonth(time_value), lut.toFirstDayOfMonth(time_value));
    EXPECT_EQ(tz.toFirstDayNumOfMonth(daynum_value), lut.toFirstDayNumOfMonth(daynum_value));
    EXPECT_EQ(tz.toFirstDayNumOfMonth(time_value), lut.toFirstDayNumOfMonth(time_value));
    EXPECT_EQ(tz.toFirstDayNumOfQuarter(daynum_value), lut.toFirstDayNumOfQuarter(daynum_value));
    EXPECT_EQ(tz.toFirstDayNumOfQuarter(time_value), lut.toFirstDayNumOfQuarter(time_value));
    EXPECT_EQ(tz.toFirstDayOfQuarter(time_value), lut.toFirstDayOfQuarter(time_value));
    EXPECT_EQ(tz.toFirstDayOfYear(time_value), lut.toFirstDayOfYear(time_value));
    EXPECT_EQ(tz.toFirstDayNumOfYear(daynum_value), lut.toFirstDayNumOfYear(daynum_value));
    EXPECT_EQ(tz.toFirstDayNumOfYear(time_value), lut.toFirstDayNumOfYear(time_value));
    EXPECT_EQ(tz.toFirstDayOfNextMonth(time_value), lut.toFirstDayOfNextMonth(time_value));
    EXPECT_EQ(tz.toFirstDayOfPrevMonth(time_value), lut.toFirstDayOfPrevMonth(time_value));
    // }
    // =============================================================================================
}

INSTANTIATE_TEST_SUITE_P(LUT0,
    TimeZone_VS_DateLUT_Test,
    ::testing::Combine(
        ::testing::Values("UTC", "Europe/Minsk"),
        ::testing::Values(
            YYYYMMDDToTimeT(19860729),
            YYYYMMDDToTimeT(19911111),
            YYYYMMDDToTimeT(20150518),
            YYYYMMDDToTimeT(20190916)
        )
    )
);


TEST(DateLUTTest, Init)
{
    // check LUT initialization time
    DateLUT::instance();
}

TEST(DateLUTTest, DISABLED_NonZeroOffset)
{
    // time_t pointing to 2169-11-13 08:00 UTC
    // NOTE: even though offest points to a certain time, LUT will have the first entry at 00:00
    const Int64 LUT_OFFSET = 6307'228'800ll;
    const Int64 FIRST_DAY_TIME_T = LUT_OFFSET - 8*3600;
    const DateLUTImpl lut("UTC", LUT_OFFSET);

//    SCOPED_TRACE(LUT_OFFSET);

//    EXPECT_EQ(lut.getDateLutMin(), FIRST_DAY_TIME_T);
//    EXPECT_EQ(lut.toGlobalDayNum(DateLUTImpl::LutDayNum(0)), 73000);

    const auto & v = lut.getValues(0);
    EXPECT_EQ(v.date, FIRST_DAY_TIME_T);
    EXPECT_EQ(v.year, 2169);
    EXPECT_EQ(v.month, 11);
    EXPECT_EQ(v.day_of_month, 13);

    const auto & v1 = lut.getValues(DayNum(1));
    EXPECT_EQ(v1.year, 2169);
    EXPECT_EQ(v1.month, 11);
    EXPECT_EQ(v1.day_of_month, 14);

    const auto & v3 = lut.getValues(DayNum(v.days_in_month));
    EXPECT_EQ(v3.year, 2169);
    EXPECT_EQ(v3.month, 12);
    EXPECT_EQ(v3.day_of_month, 14);

    EXPECT_EQ(lut.toDate(0), FIRST_DAY_TIME_T);
    EXPECT_EQ(lut.toDayNum(0), 0);
    EXPECT_EQ(lut.toNumYYYYMMDD(0), 21691113);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(0), 21691113000000);

    EXPECT_EQ(lut.timeToString(0), "2169-11-13 00:00:00");
    EXPECT_EQ(lut.dateToString(DayNum(0)), "2169-11-13");

    EXPECT_EQ(lut.toYear(0), 2169);
    EXPECT_EQ(lut.toMonth(0), 11);
    EXPECT_EQ(lut.toDayOfMonth(0), 13);
}

TEST(DateLUTTest, TimeValuesInMiddleOfRange)
{
    const DateLUTImpl lut("Europe/Minsk");
    const time_t time = 1568650811; // 2019-09-16 19:20:11 (Monday)

    EXPECT_EQ(lut.getTimeZone(), "Europe/Minsk");

    EXPECT_EQ(lut.toDate(time), 1568581200);
    EXPECT_EQ(lut.toMonth(time), 9);
    EXPECT_EQ(lut.toQuarter(time), 3);
    EXPECT_EQ(lut.toYear(time), 2019);
    EXPECT_EQ(lut.toDayOfMonth(time), 16);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), 1568581200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), DayNum(18155) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 1567285200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(18140) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(18078) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 1561928400 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 1546290000 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(17897) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 1569877200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), 1564606800 /*time_t*/);
    EXPECT_EQ(lut.daysInMonth(time), 30 /*UInt8*/);
    EXPECT_EQ(lut.toDateAndShift(time, 10), 1569445200 /*time_t*/);
    EXPECT_EQ(lut.toTime(time), 58811 /*time_t*/);
    EXPECT_EQ(lut.toHour(time), 19 /*unsigned*/);
    EXPECT_EQ(lut.toSecond(time), 11 /*unsigned*/);
    EXPECT_EQ(lut.toMinute(time), 20 /*unsigned*/);
    EXPECT_EQ(lut.toStartOfMinute(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFiveMinute(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFifteenMinutes(time), 1568650500 /*time_t*/);
    EXPECT_EQ(lut.toStartOfTenMinutes(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHour(time), 1568649600 /*time_t*/);
    EXPECT_EQ(lut.toDayNum(time), DayNum(18155) /*DayNum*/);
    EXPECT_EQ(lut.toDayOfYear(time), 259 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeWeekNum(time), 2594 /*unsigned*/);
    EXPECT_EQ(lut.toISOYear(time), 2019 /*unsigned*/);
    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), DayNum(17896) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), 1546203600 /*time_t*/);
    EXPECT_EQ(lut.toISOWeek(time), 38 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeMonthNum(time), 24237 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeQuarterNum(time), 8078 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeHourNum(time), 435736 /*time_t*/);
    EXPECT_EQ(lut.toRelativeMinuteNum(time), 26144180 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHourInterval(time, 5), 1568646000 /*time_t*/);
    EXPECT_EQ(lut.toStartOfMinuteInterval(time, 6), 1568650680 /*time_t*/);
    EXPECT_EQ(lut.toStartOfSecondInterval(time, 7), 1568650811 /*time_t*/);
    EXPECT_EQ(lut.toNumYYYYMM(time), 201909 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDD(time), 20190916 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), 20190916192011 /*UInt64*/);
    EXPECT_EQ(lut.addDays(time, 100), 1577290811 /*time_t*/);
    EXPECT_EQ(lut.addWeeks(time, 100), 1629130811 /*time_t*/);
    EXPECT_EQ(lut.addMonths(time, 100), 1831652411 /*time_t*/);
    EXPECT_EQ(lut.addQuarters(time, 100), 2357655611 /*time_t*/);
    EXPECT_EQ(lut.addYears(time, 10), 1884270011 /*time_t*/);
    EXPECT_EQ(lut.timeToString(time), "2019-09-16 19:20:11" /*std::string*/);
    EXPECT_EQ(lut.dateToString(time), "2019-09-16" /*std::string*/);
}


TEST(DateLUTTest, TimeValuesAtLeftBoderOfRange)
{
    const DateLUTImpl lut("UTC");
    const time_t time = 0;

    EXPECT_EQ(lut.getTimeZone(), "UTC");

    EXPECT_EQ(lut.toDate(time), 0);
    EXPECT_EQ(lut.toMonth(time), 1);
    EXPECT_EQ(lut.toQuarter(time), 1);
    EXPECT_EQ(lut.toYear(time), 1970);
    EXPECT_EQ(lut.toDayOfMonth(time), 1);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), 4294944000 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), DayNum(65533) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 2678400 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), 4294944000 /*time_t*/);
    EXPECT_EQ(lut.daysInMonth(time), 31 /*UInt8*/);
    EXPECT_EQ(lut.toDateAndShift(time, 10), 864000 /*time_t*/);
    EXPECT_EQ(lut.toTime(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toHour(time), 0 /*unsigned*/);
    EXPECT_EQ(lut.toSecond(time), 0 /*unsigned*/);
    EXPECT_EQ(lut.toMinute(time), 0 /*unsigned*/);
    EXPECT_EQ(lut.toStartOfMinute(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFiveMinute(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFifteenMinutes(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfTenMinutes(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHour(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toDayNum(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toDayOfYear(time), 1 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeWeekNum(time), 0 /*unsigned*/);
    EXPECT_EQ(lut.toISOYear(time), 1970 /*unsigned*/);
    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), DayNum(65533) /*DayNum*/); // ?
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), 4294944000 /*time_t*/); // ?
    EXPECT_EQ(lut.toISOWeek(time), 1 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeMonthNum(time), 23641 /*unsigned*/); // ?
    EXPECT_EQ(lut.toRelativeQuarterNum(time), 7880 /*unsigned*/); // ?
    EXPECT_EQ(lut.toRelativeHourNum(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toRelativeMinuteNum(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHourInterval(time, 5), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfMinuteInterval(time, 6), 0 /*time_t*/);
    EXPECT_EQ(lut.toStartOfSecondInterval(time, 7), 0 /*time_t*/);
    EXPECT_EQ(lut.toNumYYYYMM(time), 197001 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDD(time), 19700101 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), 19700101000000 /*UInt64*/);
    EXPECT_EQ(lut.addDays(time, 100), 8640000 /*time_t*/);
    EXPECT_EQ(lut.addWeeks(time, 100), 60480000 /*time_t*/);
    EXPECT_EQ(lut.addMonths(time, 100), 262828800 /*time_t*/);
    EXPECT_EQ(lut.addQuarters(time, 100), 788918400 /*time_t*/);
    EXPECT_EQ(lut.addYears(time, 10), 315532800 /*time_t*/);
    EXPECT_EQ(lut.timeToString(time), "1970-01-01 00:00:00" /*std::string*/);
    EXPECT_EQ(lut.dateToString(time), "1970-01-01" /*std::string*/);
}

TEST(DateLUTTest, DISABLED_TimeValuesAtRightBoderOfRange)
{
    const DateLUTImpl lut("UTC");
    const time_t time = DATE_LUT_SIZE_IN_SECONDS - 3600 * 24 * 7;

    EXPECT_EQ(lut.getTimeZone(), "UTC");

    EXPECT_EQ(lut.toDate(time), 4294857600);
    EXPECT_EQ(lut.toMonth(time), 2);
    EXPECT_EQ(lut.toQuarter(time), 1);
    EXPECT_EQ(lut.toYear(time), 2106);
    EXPECT_EQ(lut.toDayOfMonth(time), 6);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), 4294425600 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), DayNum(49704));
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 4294425600 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(49704));
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(49673) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 4291747200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 0 /*time_t*/);                        // !!! wraps around?
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(0) /*DayNum*/);             // !!! wraps around?
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 4294944000 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), 4291747200 /*time_t*/);
    EXPECT_EQ(lut.daysInMonth(time), 28 /*UInt8*/);
    EXPECT_EQ(lut.toDateAndShift(time, 10), 4294944000 /*time_t*/);
    EXPECT_EQ(lut.toTime(time), 23295 /*time_t*/);
    EXPECT_EQ(lut.toHour(time), 6 /*unsigned*/);
    EXPECT_EQ(lut.toSecond(time), 15 /*unsigned*/);
    EXPECT_EQ(lut.toMinute(time), 28 /*unsigned*/);
    EXPECT_EQ(lut.toStartOfMinute(time), 4294880880 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFiveMinute(time), 4294880700 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFifteenMinutes(time), 4294880100 /*time_t*/);
    EXPECT_EQ(lut.toStartOfTenMinutes(time), 4294880400 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHour(time), 4294879200 /*time_t*/);
    EXPECT_EQ(lut.toDayNum(time), DayNum(49709) /*DayNum*/);
    EXPECT_EQ(lut.toDayOfYear(time), 49710 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeWeekNum(time), 7101 /*unsigned*/);
    EXPECT_EQ(lut.toISOYear(time), 2106 /*unsigned*/);
    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), DayNum(65533) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), 4294944000 /*time_t*/);
    EXPECT_EQ(lut.toISOWeek(time), 7102 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeMonthNum(time), 25274 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeQuarterNum(time), 8424 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeHourNum(time), 1193022 /*time_t*/);
    EXPECT_EQ(lut.toRelativeMinuteNum(time), 71581348 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHourInterval(time, 5), 4294872000 /*time_t*/);
    EXPECT_EQ(lut.toStartOfMinuteInterval(time, 6), 4294880640 /*time_t*/);
    EXPECT_EQ(lut.toStartOfSecondInterval(time, 7), 4294880891 /*time_t*/);
    EXPECT_EQ(lut.toNumYYYYMM(time), 210602 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDD(time), 21060206 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), 21060206062815 /*UInt64*/);
    EXPECT_EQ(lut.addDays(time, 100), 4294967295 /*time_t*/);
    EXPECT_EQ(lut.addWeeks(time, 100), 4294967295 /*time_t*/);
    EXPECT_EQ(lut.addMonths(time, 100), 23295 /*time_t*/);                      // !!! wraps around
    EXPECT_EQ(lut.addQuarters(time, 100), 23295 /*time_t*/);                    // !!! wraps around
    EXPECT_EQ(lut.addYears(time, 10), 23295 /*time_t*/);                        // !!! wraps around
    EXPECT_EQ(lut.timeToString(time), "2106-02-06 06:28:15" /*std::string*/);
    EXPECT_EQ(lut.dateToString(time), "2106-02-06" /*std::string*/);
}

class DateLUTRangeTest : public ::testing::TestWithParam<std::tuple<time_t /*begin*/, time_t /*end*/, int /*step*/>>
{};

// refactored test from tests/date_lut3.cpp
TEST_P(DateLUTRangeTest, UTC)
{
    // TODO: convert to C++20 timezone-aware time values and make timezone a test parameter too.
    const auto & lut = DateLUT::instance("UTC");
    const auto & [begin, end, step] = GetParam();

    for (time_t expected_time_t = begin; expected_time_t < end; expected_time_t += step)
    {
//        SCOPED_TRACE(expected_time_t);

        tm tm;
        ASSERT_NE(gmtime_r(&expected_time_t, &tm), nullptr);

        char expected_time_string[96] = {'\0'};
        snprintf(expected_time_string, sizeof(expected_time_string),
                 "%04d-%02d-%02d %02d:%02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

//        SCOPED_TRACE(expected_time_string);

        EXPECT_EQ(tm.tm_year + 1900, lut.toYear(expected_time_t));
        EXPECT_EQ(tm.tm_mon + 1, lut.toMonth(expected_time_t));
        EXPECT_EQ(tm.tm_mday, lut.toDayOfMonth(expected_time_t));
        EXPECT_EQ(tm.tm_wday, lut.toDayOfWeek(expected_time_t) % 7); // tm.tm_wday Sunday is 0, while for DateLUTImpl it is 7
        EXPECT_EQ(tm.tm_yday + 1, lut.toDayOfYear(expected_time_t));
        EXPECT_EQ(tm.tm_hour, lut.toHour(expected_time_t));
        EXPECT_EQ(tm.tm_min, lut.toMinute(expected_time_t));
        EXPECT_EQ(tm.tm_sec, lut.toSecond(expected_time_t));

        EXPECT_EQ(expected_time_t, lut.makeDateTime(
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec));

        EXPECT_EQ(expected_time_string, lut.timeToString(expected_time_t));

        if (::testing::Test::HasFailure())
        {
            // it makes sense to let test execute all checks above to simplify debugging,
            // but once we've found a bad apple, no need to dig deeper.
            std::cerr << "Breaking early due to failures..." << std::endl;
            break;
        }
    }
}

INSTANTIATE_TEST_SUITE_P(Year2010,
    DateLUTRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        // Values from tests/date_lut3.cpp
        {YYYYMMDDToTimeT(20101031), YYYYMMDDToTimeT(20101101), 15 * 60},
        {YYYYMMDDToTimeT(20100328), YYYYMMDDToTimeT(20100330), 15 * 60}
    })
);

INSTANTIATE_TEST_SUITE_P(Year1970,
    DateLUTRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(19700101), YYYYMMDDToTimeT(19700201), 15 * 60},
        // 11 was chosen as a number which can't divide product of 2-combinarions of (7, 24, 60),
        // to reduce likelehood of hitting same hour/minute/second values for different days.
        // + 12 is just to make sure that last day is covered fully.
        {0, 0 + 11 * 3600 * 24 + 12, 11},
    })
);

class TimeZoneTest : public ::testing::Test
{};

TEST_F(TimeZoneTest, getLUTIndex)
{
    ASSERT_EQ(YYYYMMDDToTimeT(1970'01'01), 0);
    ASSERT_EQ(YYYYMMDDToTimeT(1969'12'31), -3600 * 24);
    ASSERT_EQ(YYYYMMDDToTimeT(1970'01'02), 3600 * 24);

    const TimeZoneImpl tz("UTC");

    // time_t
    EXPECT_EQ(tz.getLUTIndex(0), 0);
    EXPECT_EQ(tz.getLUTIndex(1), 0);
    EXPECT_EQ(tz.getLUTIndex(DATE_LUT_SIZE_IN_SECONDS - 1), 0);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(1970'01'01)), 0);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(2105'01'01)), 0);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(2105'12'31)), 0);

    EXPECT_EQ(tz.getLUTIndex(-1), -1);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(1960'01'01)), -1);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(1969'01'01)), -1);

    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(1800'01'01)), -2);

    EXPECT_EQ(tz.getLUTIndex(DATE_LUT_SIZE_IN_SECONDS), 1);
    EXPECT_EQ(tz.getLUTIndex(DATE_LUT_SIZE_IN_SECONDS + 1), 1);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(2200'01'01)), 1);
    EXPECT_EQ(tz.getLUTIndex(YYYYMMDDToTimeT(2400'01'01)), 3);

    // By DayNum
    const auto SECONDS_IN_DAY = 3600 * 24;
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(-1)), -1);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(0)), 0);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(1)), 0);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum((DATE_LUT_SIZE_IN_SECONDS + SECONDS_IN_DAY) / SECONDS_IN_DAY)), 1);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(YYYYMMDDToTimeT(2200'01'01) / SECONDS_IN_DAY)), 1);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(YYYYMMDDToTimeT(2400'01'01) / SECONDS_IN_DAY)), 3);
    EXPECT_EQ(tz.getLUTIndex(GlobalDayNum(YYYYMMDDToTimeT(2411'01'01) / SECONDS_IN_DAY)), 3);

    // YYYY MM DD
    EXPECT_EQ(tz.getLUTIndex(1833, 11, 25), -1);
    EXPECT_EQ(tz.getLUTIndex(1969, 12, 31), -1);
    EXPECT_EQ(tz.getLUTIndex(1969, 01, 01), -1);

    EXPECT_EQ(tz.getLUTIndex(1970, 01, 01), 0);
    EXPECT_EQ(tz.getLUTIndex(1970, 01, 02), 0);
    EXPECT_EQ(tz.getLUTIndex(2105, 12, 31), 0);

    EXPECT_EQ(tz.getLUTIndex(2200, 01, 01), 1);
    EXPECT_EQ(tz.getLUTIndex(2400, 01, 01), 3);
}


TEST_F(TimeZoneTest, TimeValuesInMiddleOfRange)
{
    const TimeZoneImpl lut("Europe/Minsk");
    const time_t time = 1568650811; // 2019-09-16 19:20:11 (Monday)

    EXPECT_EQ(lut.getTimeZone(), "Europe/Minsk");

    EXPECT_EQ(lut.toDate(time), 1568581200);
    EXPECT_EQ(lut.toDate(GlobalDayNum(18155)), 1568581200);
    EXPECT_EQ(lut.toMonth(time), 9);
    EXPECT_EQ(lut.toQuarter(time), 3);
    EXPECT_EQ(lut.toYear(time), 2019);
    EXPECT_EQ(lut.toDayOfMonth(time), 16);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), 1568581200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), DayNum(18155) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 1567285200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(18140) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(18078) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 1561928400 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 1546290000 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(17897) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 1569877200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), 1564606800 /*time_t*/);
    EXPECT_EQ(lut.daysInMonth(time), 30 /*UInt8*/);
    EXPECT_EQ(lut.toDateAndShift(time, 10), 1569445200 /*time_t*/);
    EXPECT_EQ(lut.toTime(time), 58811 /*time_t*/);
    EXPECT_EQ(lut.toHour(time), 19 /*unsigned*/);
    EXPECT_EQ(lut.toSecond(time), 11 /*unsigned*/);
    EXPECT_EQ(lut.toMinute(time), 20 /*unsigned*/);
    EXPECT_EQ(lut.toStartOfMinute(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFiveMinute(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFifteenMinutes(time), 1568650500 /*time_t*/);
    EXPECT_EQ(lut.toStartOfTenMinutes(time), 1568650800 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHour(time), 1568649600 /*time_t*/);
    EXPECT_EQ(lut.toDayNum(time), DayNum(18155) /*DayNum*/);
    EXPECT_EQ(lut.toDayOfYear(time), 259 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeWeekNum(time), 2594 /*unsigned*/);
    EXPECT_EQ(lut.toISOYear(time), 2019 /*unsigned*/);
    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), DayNum(17896) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), 1546203600 /*time_t*/);
    EXPECT_EQ(lut.toISOWeek(time), 38 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeMonthNum(time), 24237 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeQuarterNum(time), 8078 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeHourNum(time), 435736 /*time_t*/);
    EXPECT_EQ(lut.toRelativeMinuteNum(time), 26144180 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHourInterval(time, 5), 1568646000 /*time_t*/);
    EXPECT_EQ(lut.toStartOfMinuteInterval(time, 6), 1568650680 /*time_t*/);
    EXPECT_EQ(lut.toStartOfSecondInterval(time, 7), 1568650811 /*time_t*/);
    EXPECT_EQ(lut.toNumYYYYMM(time), 201909 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDD(time), 20190916 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), 20190916192011 /*UInt64*/);
    EXPECT_EQ(lut.addDays(time, 100), 1577290811 /*time_t*/);
    EXPECT_EQ(lut.addWeeks(time, 100), 1629130811 /*time_t*/);
    EXPECT_EQ(lut.addMonths(time, 100), 1831652411 /*time_t*/);
    EXPECT_EQ(lut.addQuarters(time, 100), 2357655611 /*time_t*/);
    EXPECT_EQ(lut.addYears(time, 10), 1884270011 /*time_t*/);
    EXPECT_EQ(lut.timeToString(time), "2019-09-16 19:20:11" /*std::string*/);
    EXPECT_EQ(lut.dateToString(time), "2019-09-16" /*std::string*/);
}


class TimeZoneRangeTest : public ::testing::TestWithParam<std::tuple<time_t /*begin*/, time_t /*end*/, int /*step*/>>
{};

TEST_P(TimeZoneRangeTest, UTC)
{
    // TODO: convert to C++20 timezone-aware time values and make a timezone a test parameter too.
    const auto & tz = TimeZoneImpl("UTC");
    const auto & [begin, end, step] = GetParam();

    // Precautions agains invalid test params that may cause test to run infinite time.
    ASSERT_NE(0, step);
    ASSERT_EQ(std::signbit(end - begin), std::signbit(step));

    auto prev_lut_index = tz.getLUTIndex(begin);
    int i = 0;
    const ssize_t total_steps = (end - begin) / step;
    for (time_t expected_time_t = begin; begin < end ? expected_time_t < end : expected_time_t > end; expected_time_t += step, ++i)
    {
        const auto lut_index = tz.getLUTIndex(expected_time_t);
//        const auto & lut = tz.getLUTByIndex(lut_index);
//        SCOPED_TRACE(toString("LUT index:", lut_index, " lut:", lut.getDateLutMin(), ", ", lut.getDayNumLutMin()));

        tm tm;
        ASSERT_NE(gmtime_r(&expected_time_t, &tm), nullptr);

        char expected_time_string[100] = {'\0'};
        snprintf(expected_time_string, sizeof(expected_time_string),
                 "%04d-%02d-%02d %02d:%02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

//        SCOPED_TRACE(toString("#", i, " time value: ", expected_time_t));

        const auto report_interval = 1'567'000LL;
        if (lut_index != prev_lut_index
            || (total_steps / report_interval > 1
                && (i % report_interval == 0 || i >= total_steps || i == 0)))
        {
            std::cerr << "!!!!! " << i << " of " << total_steps << " (" << (1.0 * i) / total_steps * 100 << "%)"
                      << " : " << expected_time_string
                      << " LUT INDEX: " << tz.getLUTIndex(expected_time_t) << std::endl;
        }

//        SCOPED_TRACE(toString("time: ", expected_time_string));

        EXPECT_EQ(tm.tm_year + 1900, tz.toYear(expected_time_t));
        EXPECT_EQ(tm.tm_mon + 1, tz.toMonth(expected_time_t));
        EXPECT_EQ(tm.tm_mday, tz.toDayOfMonth(expected_time_t));
        EXPECT_EQ(tm.tm_wday, tz.toDayOfWeek(expected_time_t) % 7); // tm.tm_wday Sunday is 0, while for DateLUTImpl it is 7
        EXPECT_EQ(tm.tm_yday + 1, tz.toDayOfYear(expected_time_t));
        EXPECT_EQ(tm.tm_hour, tz.toHour(expected_time_t));
        EXPECT_EQ(tm.tm_min, tz.toMinute(expected_time_t));
        EXPECT_EQ(tm.tm_sec, tz.toSecond(expected_time_t));

        EXPECT_EQ(expected_time_t, tz.makeDateTime(
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec))
                << "With args\t" << tm.tm_year + 1900 << ", "
                << tm.tm_mon + 1 << ", "
                << tm.tm_mday << ", "
                << tm.tm_hour << ", "
                << tm.tm_min << ", "
                << tm.tm_sec;

        EXPECT_EQ(expected_time_string, tz.timeToString(expected_time_t));

        if (::testing::Test::HasFailure())
        {
            // it makes sense to let test execute all checks above to simplify debugging,
            // but once we've found a bad apple, no need to dig deeper.
            std::cerr << "Breaking early due to failures..." << std::endl;
            break;
        }

        prev_lut_index = lut_index;
    }
}


// LUT index 0
INSTANTIATE_TEST_SUITE_P(Year2010,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        // Values from tests/date_lut3.cpp
        {YYYYMMDDToTimeT(2010'10'31), YYYYMMDDToTimeT(2010'11'01), 15 * 60},
        {YYYYMMDDToTimeT(2010'03'28), YYYYMMDDToTimeT(2010'03'30), 15 * 60},

        // beginning of the year
        {YYYYMMDDToTimeT(2010'01'01), YYYYMMDDToTimeT(2010'03'01), 15 * 60},
        // end of the year
        {YYYYMMDDToTimeT(2010'11'01), YYYYMMDDToTimeT(2010'12'31), 15 * 60},
    })
);

// LUT index 0
INSTANTIATE_TEST_SUITE_P(Year1970,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(1971'01'01), YYYYMMDDToTimeT(1971'03'01), 15 * 60},
        // 11 was chosen as a number which can't divide product of 2-combinarions of (7, 24, 60),
        // to reduce likelehood of hitting same hour/minute/second values for different days.
        // + 12 is just to make sure that last day is covered fully.
        {0, 0 + 11 * 3600 * 24 + 12, 11},

        // beginning of the year
        {YYYYMMDDToTimeT(1970'01'01), YYYYMMDDToTimeT(1970'03'01), 15 * 60},
        // end of the year
        {YYYYMMDDToTimeT(1970'11'01), YYYYMMDDToTimeT(1970'12'31), 15 * 60},
    })
);


// LUT index -1
INSTANTIATE_TEST_SUITE_P(Year1960,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(1960'01'01), YYYYMMDDToTimeT(1960'03'01), 15 * 60},
        {YYYYMMDDToTimeT(1960'11'01), YYYYMMDDToTimeT(1960'12'31), 15 * 60},
    })
);


// LUT index -1
INSTANTIATE_TEST_SUITE_P(Year1969,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        // beginning of the year
        {YYYYMMDDToTimeT(1969'01'01), YYYYMMDDToTimeT(1969'03'01), 15 * 60},
        // end of the year
        {YYYYMMDDToTimeT(1969'11'01), YYYYMMDDToTimeT(1969'12'31), 15 * 60},
    })
);

// LUT index -1
INSTANTIATE_TEST_SUITE_P(Year1900,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(1900'01'01), YYYYMMDDToTimeT(1900'03'01), 15 * 60},
        {YYYYMMDDToTimeT(1900'11'01), YYYYMMDDToTimeT(1900'12'31), 15 * 60},
    })
);

// LUT index -2
INSTANTIATE_TEST_SUITE_P(Year1800,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(18000101), YYYYMMDDToTimeT(18000201), 15 * 60},
        {YYYYMMDDToTimeT(18001101), YYYYMMDDToTimeT(18001231), 15 * 60},
    })
);

// Border between LUT with indices -2 and -1
INSTANTIATE_TEST_SUITE_P(Year1833,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(1833'01'01), YYYYMMDDToTimeT(1833'02'01), 15 * 60},
        {YYYYMMDDToTimeT(1833'11'01), YYYYMMDDToTimeT(1833'12'31), 15 * 60},
    })
);

// LUT index 1
INSTANTIATE_TEST_SUITE_P(Year2200,
    TimeZoneRangeTest,
    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
        {YYYYMMDDToTimeT(22000101), YYYYMMDDToTimeT(22000201), 15 * 60},
        {YYYYMMDDToTimeT(22001101), YYYYMMDDToTimeT(22001231), 15 * 60},
    })
);

//// LUT index -2 to +2
//INSTANTIATE_TEST_SUITE_P(Years1800_2300,
//    TimeZoneRangeTest,
//    ::testing::ValuesIn(std::initializer_list<typename DateLUTRangeTest::ParamType>{
//        {YYYYMMDDToTimeT(1800'01'01), YYYYMMDDToTimeT(2300'12'31), 86400 + 113},
//    })
//);
