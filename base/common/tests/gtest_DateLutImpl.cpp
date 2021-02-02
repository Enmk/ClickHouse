#include <common/DateLUT.h>
#include <common/DateLUTImpl.h>

#include <gtest/gtest.h>

#include <string>
#include <cctz/time_zone.h>
#include <cctz/civil_time.h>

/// For the expansion of gtest macros.
#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wused-but-marked-unused"
    #pragma clang diagnostic ignored "-Wunused-member-function"
#endif

// All timezones present at build time and embedded into CH binary.
extern const char * auto_time_zones[];

namespace
{

cctz::civil_day YYYYMMDDToDay(unsigned value)
{
    return cctz::civil_day(
        value / 10000,         // year
        (value % 10000) / 100, // month
        value % 100);          // day
}

cctz::civil_second YYYYMMDDHMMSSToSecond(std::uint64_t value)
{
    return cctz::civil_second(
            value / 10000000000,
            value / 100000000 % 100,
            value / 1000000 % 100,
            value / 10000 % 100,
            value / 100 % 100,
            value % 100);
}


std::vector<const char*> allTimezones()
{
    std::vector<const char*> result;

    auto timezone_name = auto_time_zones;
    while (*timezone_name)
    {
        result.push_back(*timezone_name);
        ++timezone_name;
    }

    return result;
}

struct FailuresCount
{
    size_t non_fatal = 0;
    size_t fatal = 0;
    size_t total = 0;
};

FailuresCount countFailures(const ::testing::TestResult & test_result)
{
    FailuresCount failures{0, 0, 0};
    const size_t count = test_result.total_part_count();
    for (size_t i = 0; i < count; ++i)
    {
        const auto & part = test_result.GetTestPartResult(i);
        if (part.nonfatally_failed())
        {
            ++failures.non_fatal;
            ++failures.total;
        }
        if (part.fatally_failed())
        {
            ++failures.fatal;
            ++failures.total;
        }
    }

    return failures;
}

class FullRangeTimeZone
{
    const cctz::time_zone & time_zone;
    const cctz::civil_second epoch_start;

    template <typename ResultCCTZTimeType = cctz::civil_second>
    inline ResultCCTZTimeType toCCTZ(time_t t) const
    {
        return ResultCCTZTimeType(cctz::convert(std::chrono::system_clock::from_time_t(t), time_zone));
    }

    template <typename ResultCCTZTimeType = cctz::civil_day>
    inline ResultCCTZTimeType toCCTZ(DayNum d) const
    {
        return ResultCCTZTimeType(cctz::civil_day(epoch_start) + d);
    }

    // Only for cctz::civil_x types
    template <typename CCTZTimeType>
    inline time_t toTimeTFromCCTZ(CCTZTimeType && cctz_time) const
    {
        static_assert(sizeof(cctz_time.year()) > 0);
        return std::chrono::system_clock::to_time_t(cctz::convert(cctz::civil_day(cctz_time), time_zone));
    }

    // Only for cctz::civil_x types
    template <typename CCTZTimeType>
    DayNum toDayNumFromCCTZ(CCTZTimeType && cctz_time) const
    {
        return DayNum(cctz::detail::impl::ymd_ord(cctz_time.year(), cctz_time.month(), cctz_time.day()));
//        return DayNum(sinceEpoch<cctz::civil_day>(cctz_time));
    }

public:
    FullRangeTimeZone(const cctz::time_zone & time_zone_)
        : time_zone(time_zone_),
          epoch_start(cctz::convert(std::chrono::system_clock::from_time_t(0), time_zone))
    {}

    // Get seconds, minutes, hours, days, months or years since the epoch:
    // sinceEpoch<cctz::civil_day>(value); // seconds to `value` since `epoch_start`
    template <typename DiffUnitsType, typename T>
    int64_t sinceEpoch(const T & datetime_unit) const
    {
        return DiffUnitsType(datetime_unit) - DiffUnitsType(epoch_start);
    }

    const std::string getTimeZone() const { return time_zone.name(); }
    inline time_t toDate(time_t t) const
    {
        const auto datetime = toCCTZ(t);
        return t - (datetime.hour() * 86400 + datetime.minute() * 60 + datetime.second());
    }

    inline unsigned toMonth(time_t t) const { return toCCTZ(t).month(); }
    inline unsigned toQuarter(time_t t) const { return (toMonth(t) - 1) / 3 + 1; }
    inline unsigned toYear(time_t t) const { return toCCTZ(t).year(); }
    inline unsigned toDayOfWeek(time_t t) const { return static_cast<int>(cctz::get_weekday(toCCTZ(t))) + 1; }
    inline unsigned toDayOfMonth(time_t t) const { return toCCTZ(t).day(); }
    /// Round down to start of monday.
    inline time_t toFirstDayOfWeek(time_t t) const
    {
        const auto datetime = cctz::civil_day(toCCTZ(t));
        return toTimeTFromCCTZ(datetime - static_cast<int>(cctz::get_weekday(datetime)));
    }

    inline DayNum toFirstDayNumOfWeek(DayNum d) const
    {
        const auto date = toCCTZ(d);
        return toDayNumFromCCTZ(date - static_cast<int>(cctz::get_weekday(date)));
    }

    inline DayNum toFirstDayNumOfWeek(time_t t) const
    {
        const auto datetime = toCCTZ<cctz::civil_day>(t);
        return toDayNumFromCCTZ(datetime - static_cast<int>(cctz::get_weekday(datetime)));
    }

    /// Round down to start of month.
    inline time_t toFirstDayOfMonth(time_t t) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_month>(t));
    }

    inline DayNum toFirstDayNumOfMonth(DayNum d) const
    {
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_month>(d));
    }

    inline DayNum toFirstDayNumOfMonth(time_t t) const
    {
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_month>(t));
    }

    /// Round down to start of quarter.
    inline DayNum toFirstDayNumOfQuarter(DayNum d) const
    {
        auto day = toCCTZ(d);
        return toDayNumFromCCTZ(day - (day.month() - 1) % 3);
    }

    inline DayNum toFirstDayNumOfQuarter(time_t t) const
    {
        auto day = toCCTZ<cctz::civil_month>(t);
        return toDayNumFromCCTZ(day - (day.month() - 1) % 3);
    }

    inline time_t toFirstDayOfQuarter(time_t t) const
    {
        auto day = toCCTZ<cctz::civil_month>(t);
        return toTimeTFromCCTZ(day - (day.month() - 1) % 3);
    }

    /// Round down to start of year.
    inline time_t toFirstDayOfYear(time_t t) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_year>(t));
    }

    inline DayNum toFirstDayNumOfYear(DayNum d) const
    {
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_year>(d));
    }

    inline DayNum toFirstDayNumOfYear(time_t t) const
    {
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_year>(t));
    }

    inline time_t toFirstDayOfNextMonth(time_t t) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_month>(t) + 1);
    }

    inline time_t toFirstDayOfPrevMonth(time_t t) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_month>(t) - 1);
    }

    inline UInt8 daysInMonth(DayNum d) const
    {
        const auto day = toCCTZ<cctz::civil_day>(d);
        return cctz::detail::impl::days_per_month(day.year(), day.month());
    }

    inline UInt8 daysInMonth(time_t t) const
    {
        const auto day = toCCTZ<cctz::civil_day>(t);
        return cctz::detail::impl::days_per_month(day.year(), day.month());
    }

    inline UInt8 daysInMonth(UInt16 year, UInt8 month) const
    {
        return cctz::detail::impl::days_per_month(year, month);
    }

    /** Round to start of day, then shift for specified amount of days.
      */
    inline time_t toDateAndShift(time_t t, Int32 days) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_day>(t) + days);
    }

//    inline time_t toTime(time_t t) const
//    {
//        DayNum index = findIndex(t);

//        if (unlikely(index == 0))
//            return t + offset_at_start_of_epoch;

//        time_t res = t - lut[index].date;

//        if (res >= lut[index].time_at_offset_change)
//            res += lut[index].amount_of_offset_change;

//        return res - offset_at_start_of_epoch; /// Starting at 1970-01-01 00:00:00 local time.
//    }

    inline unsigned toHour(time_t t) const
    {
        return toCCTZ(t).hour();
    }

    /** Only for time zones with/when offset from UTC is multiple of five minutes.
      * This is true for all time zones: right now, all time zones have an offset that is multiple of 15 minutes.
      *
      * "By 1929, most major countries had adopted hourly time zones. Nepal was the last
      *  country to adopt a standard offset, shifting slightly to UTC+5:45 in 1986."
      * - https://en.wikipedia.org/wiki/Time_zone#Offsets_from_UTC
      *
      * Also please note, that unix timestamp doesn't count "leap seconds":
      *  each minute, with added or subtracted leap second, spans exactly 60 unix timestamps.
      */

    inline unsigned toSecond(time_t t) const { return toCCTZ(t).second(); }

    inline unsigned toMinute(time_t t) const
    {
        return toCCTZ(t).minute();
    }

    inline time_t toStartOfMinute(time_t t) const { return t / 60 * 60; }
    inline time_t toStartOfFiveMinute(time_t t) const { return t / 300 * 300; }
    inline time_t toStartOfFifteenMinutes(time_t t) const { return t / 900 * 900; }
    inline time_t toStartOfTenMinutes(time_t t) const { return t / 600 * 600; }

    inline time_t toStartOfHour(time_t t) const
    {
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_hour>(t));
    }

    /** Number of calendar day since the beginning of UNIX epoch (1970-01-01 is zero)
      * We use just two bytes for it. It covers the range up to 2105 and slightly more.
      *
      * This is "calendar" day, it itself is independent of time zone
      * (conversion from/to unix timestamp will depend on time zone,
      *  because the same calendar day starts/ends at different timestamps in different time zones)
      */

    inline DayNum toDayNum(time_t t) const { return toDayNumFromCCTZ(toCCTZ(t)); }
    inline time_t fromDayNum(DayNum d) const { return toTimeTFromCCTZ(toCCTZ(d)); }

    inline time_t toDate(DayNum d) const { return fromDayNum(d); }
    inline unsigned toMonth(DayNum d) const { return toCCTZ(d).month(); }
    inline unsigned toQuarter(DayNum d) const { return (toMonth(d) - 1) / 3 + 1; }
    inline unsigned toYear(DayNum d) const { return toCCTZ(d).year(); }
    inline unsigned toDayOfWeek(DayNum d) const { return static_cast<int>(cctz::get_weekday(toCCTZ(d))) + 1; }
    inline unsigned toDayOfMonth(DayNum d) const { return toCCTZ(d).day(); }
    inline unsigned toDayOfYear(DayNum d) const { return cctz::get_yearday(toCCTZ(d)); }

    inline unsigned toDayOfYear(time_t t) const { return cctz::get_yearday(toCCTZ(t)); }

    /// Number of week from some fixed moment in the past. Week begins at monday.
    /// (round down to monday and divide DayNum by 7; we made an assumption,
    ///  that in domain of the function there was no weeks with any other number of days than 7)
    inline unsigned toRelativeWeekNum(DayNum d) const
    {
        /// We add 8 to avoid underflow at beginning of unix epoch.
        return (d + 8 - toDayOfWeek(d)) / 7;
    }

    inline unsigned toRelativeWeekNum(time_t t) const
    {
        return toRelativeWeekNum(toDayNum(t));
    }

    /// Get year that contains most of the current week. Week begins at monday.
    inline unsigned toISOYear(DayNum d) const
    {
        /// That's effectively the year of thursday of current week.
        return toYear(DayNum(d + 4 - toDayOfWeek(d)));
    }

    inline unsigned toISOYear(time_t t) const
    {
        return toISOYear(toDayNum(t));
    }

//    /// ISO year begins with a monday of the week that is contained more than by half in the corresponding calendar year.
//    /// Example: ISO year 2019 begins at 2018-12-31. And ISO year 2017 begins at 2017-01-02.
//    /// https://en.wikipedia.org/wiki/ISO_week_date
//    inline DayNum toFirstDayNumOfISOYear(DayNum d) const
//    {
//        auto iso_year = toISOYear(d);

//        DayNum first_day_of_year = years_lut[iso_year - DATE_LUT_MIN_YEAR];
//        auto first_day_of_week_of_year = lut[first_day_of_year].day_of_week;

//        return DayNum(first_day_of_week_of_year <= 4
//            ? first_day_of_year + 1 - first_day_of_week_of_year
//            : first_day_of_year + 8 - first_day_of_week_of_year);
//    }

//    inline DayNum toFirstDayNumOfISOYear(time_t t) const
//    {
//        return toFirstDayNumOfISOYear(toDayNum(t));
//    }

//    inline time_t toFirstDayOfISOYear(time_t t) const
//    {
//        return fromDayNum(toFirstDayNumOfISOYear(t));
//    }

//    /// ISO 8601 week number. Week begins at monday.
//    /// The week number 1 is the first week in year that contains 4 or more days (that's more than half).
//    inline unsigned toISOWeek(DayNum d) const
//    {
//        return 1 + DayNum(toFirstDayNumOfWeek(d) - toFirstDayNumOfISOYear(d)) / 7;
//    }

//    inline unsigned toISOWeek(time_t t) const
//    {
//        return toISOWeek(toDayNum(t));
//    }

//    /*
//      The bits in week_mode has the following meaning:
//       WeekModeFlag::MONDAY_FIRST (0)  If not set Sunday is first day of week
//                      If set Monday is first day of week
//       WeekModeFlag::YEAR (1) If not set Week is in range 0-53

//        Week 0 is returned for the the last week of the previous year (for
//        a date at start of january) In this case one can get 53 for the
//        first week of next year.  This flag ensures that the week is
//        relevant for the given year. Note that this flag is only
//        relevant if WeekModeFlag::JANUARY is not set.

//                  If set Week is in range 1-53.

//        In this case one may get week 53 for a date in January (when
//        the week is that last week of previous year) and week 1 for a
//        date in December.

//      WeekModeFlag::FIRST_WEEKDAY (2) If not set Weeks are numbered according
//                        to ISO 8601:1988
//                  If set The week that contains the first
//                        'first-day-of-week' is week 1.

//      WeekModeFlag::NEWYEAR_DAY (3) If not set no meaning
//                  If set The week that contains the January 1 is week 1.
//                            Week is in range 1-53.
//                            And ignore WeekModeFlag::YEAR, WeekModeFlag::FIRST_WEEKDAY

//        ISO 8601:1988 means that if the week containing January 1 has
//        four or more days in the new year, then it is week 1;
//        Otherwise it is the last week of the previous year, and the
//        next week is week 1.
//    */
//    inline YearWeek toYearWeek(DayNum d, UInt8 week_mode) const
//    {
//        bool newyear_day_mode = week_mode & static_cast<UInt8>(WeekModeFlag::NEWYEAR_DAY);
//        week_mode = check_week_mode(week_mode);
//        bool monday_first_mode = week_mode & static_cast<UInt8>(WeekModeFlag::MONDAY_FIRST);
//        bool week_year_mode = week_mode & static_cast<UInt8>(WeekModeFlag::YEAR);
//        bool first_weekday_mode = week_mode & static_cast<UInt8>(WeekModeFlag::FIRST_WEEKDAY);

//        // Calculate week number of WeekModeFlag::NEWYEAR_DAY mode
//        if (newyear_day_mode)
//        {
//            return toYearWeekOfNewyearMode(d, monday_first_mode);
//        }

//        YearWeek yw(toYear(d), 0);
//        UInt16 days = 0;
//        UInt16 daynr = makeDayNum(yw.first, toMonth(d), toDayOfMonth(d));
//        UInt16 first_daynr = makeDayNum(yw.first, 1, 1);

//        // 0 for monday, 1 for tuesday ...
//        // get weekday from first day in year.
//        UInt16 weekday = calc_weekday(DayNum(first_daynr), !monday_first_mode);

//        if (toMonth(d) == 1 && toDayOfMonth(d) <= static_cast<UInt32>(7 - weekday))
//        {
//            if (!week_year_mode && ((first_weekday_mode && weekday != 0) || (!first_weekday_mode && weekday >= 4)))
//                return yw;
//            week_year_mode = 1;
//            (yw.first)--;
//            first_daynr -= (days = calc_days_in_year(yw.first));
//            weekday = (weekday + 53 * 7 - days) % 7;
//        }

//        if ((first_weekday_mode && weekday != 0) || (!first_weekday_mode && weekday >= 4))
//            days = daynr - (first_daynr + (7 - weekday));
//        else
//            days = daynr - (first_daynr - weekday);

//        if (week_year_mode && days >= 52 * 7)
//        {
//            weekday = (weekday + calc_days_in_year(yw.first)) % 7;
//            if ((!first_weekday_mode && weekday < 4) || (first_weekday_mode && weekday == 0))
//            {
//                (yw.first)++;
//                yw.second = 1;
//                return yw;
//            }
//        }
//        yw.second = days / 7 + 1;
//        return yw;
//    }

//    /// Calculate week number of WeekModeFlag::NEWYEAR_DAY mode
//    /// The week number 1 is the first week in year that contains January 1,
//    inline YearWeek toYearWeekOfNewyearMode(DayNum d, bool monday_first_mode) const
//    {
//        YearWeek yw(0, 0);
//        UInt16 offset_day = monday_first_mode ? 0U : 1U;

//        // Checking the week across the year
//        yw.first = toYear(DayNum(d + 7 - toDayOfWeek(DayNum(d + offset_day))));

//        DayNum first_day = makeDayNum(yw.first, 1, 1);
//        DayNum this_day = d;

//        if (monday_first_mode)
//        {
//            // Rounds down a date to the nearest Monday.
//            first_day = toFirstDayNumOfWeek(first_day);
//            this_day = toFirstDayNumOfWeek(d);
//        }
//        else
//        {
//            // Rounds down a date to the nearest Sunday.
//            if (toDayOfWeek(first_day) != 7)
//                first_day = DayNum(first_day - toDayOfWeek(first_day));
//            if (toDayOfWeek(d) != 7)
//                this_day = DayNum(d - toDayOfWeek(d));
//        }
//        yw.second = (this_day - first_day) / 7 + 1;
//        return yw;
//    }

    /**
     * get first day of week with week_mode, return Sunday or Monday
     */
    inline DayNum toFirstDayNumOfWeek(DayNum d, UInt8 week_mode) const
    {
        const bool monday_first_mode = week_mode & static_cast<UInt8>(WeekModeFlag::MONDAY_FIRST);
        const auto first_day = monday_first_mode ? cctz::weekday::monday : cctz::weekday::sunday;
        return toDayNumFromCCTZ(cctz::prev_weekday(toCCTZ(d) + 1, first_day));
    }

//    /*
//     * check and change mode to effective
//     */
//    inline UInt8 check_week_mode(UInt8 mode) const
//    {
//        UInt8 week_format = (mode & 7);
//        if (!(week_format & static_cast<UInt8>(WeekModeFlag::MONDAY_FIRST)))
//            week_format ^= static_cast<UInt8>(WeekModeFlag::FIRST_WEEKDAY);
//        return week_format;
//    }

    /*
     * Calc weekday from d
     * Returns 0 for monday, 1 for tuesday ...
     */
    inline unsigned calc_weekday(DayNum d, bool sunday_first_day_of_week) const
    {
        if (!sunday_first_day_of_week)
            return toDayOfWeek(d) - 1;
        else
            return toDayOfWeek(DayNum(d + 1)) - 1;
    }

    /* Calc days in one year. */
    inline unsigned calc_days_in_year(UInt16 year) const
    {
        return ((year & 3) == 0 && (year % 100 || (year % 400 == 0 && year)) ? 366 : 365);
    }

    /// Number of month from some fixed moment in the past (year * 12 + month)
    inline unsigned toRelativeMonthNum(DayNum d) const
    {
        const auto date = toCCTZ<cctz::civil_month>(d);
        return date.year() * 12 + date.month();
    }

    inline unsigned toRelativeMonthNum(time_t t) const
    {
        const auto date = toCCTZ<cctz::civil_month>(t);
        return date.year() * 12 + date.month();
    }

    inline unsigned toRelativeQuarterNum(DayNum d) const
    {
        return (toRelativeMonthNum(d) - 1) / 3;
    }

    inline unsigned toRelativeQuarterNum(time_t t) const
    {
        return (toRelativeMonthNum(t) - 1) / 3;
    }

    /// We count all hour-length intervals, unrelated to offset changes.
    inline time_t toRelativeHourNum(time_t t) const
    {
        return sinceEpoch<cctz::civil_hour>(toCCTZ(t));
    }

    inline time_t toRelativeHourNum(DayNum d) const
    {
        return sinceEpoch<cctz::civil_hour>(toCCTZ(d));
    }

    inline time_t toRelativeMinuteNum(time_t t) const
    {
        return t / 60;
    }

    inline time_t toRelativeMinuteNum(DayNum d) const
    {
        return sinceEpoch<cctz::civil_minute>(toCCTZ(d));
    }

    inline DayNum toStartOfYearInterval(DayNum d, UInt64 years) const
    {
//        if (years == 1)
//            return toFirstDayNumOfYear(d);
//        return years_lut[(lut[d].year - DATE_LUT_MIN_YEAR) / years * years];
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_year>(d) + years);
    }

//    inline DayNum toStartOfQuarterInterval(DayNum d, UInt64 quarters) const
//    {
//        if (quarters == 1)
//            return toFirstDayNumOfQuarter(d);
//        return toStartOfMonthInterval(d, quarters * 3);
//    }

    inline DayNum toStartOfMonthInterval(DayNum d, UInt64 months) const
    {
//        if (months == 1)
//            return toFirstDayNumOfMonth(d);
//        const auto & date = lut[d];
//        UInt32 month_total_index = (date.year - DATE_LUT_MIN_YEAR) * 12 + date.month - 1;
//        return years_months_lut[month_total_index / months * months];
        return toDayNumFromCCTZ(toCCTZ<cctz::civil_month>(d) + months);
    }

//    inline DayNum toStartOfWeekInterval(DayNum d, UInt64 weeks) const
//    {
//        if (weeks == 1)
//            return toFirstDayNumOfWeek(d);
//        UInt64 days = weeks * 7;
//        // January 1st 1970 was Thursday so we need this 4-days offset to make weeks start on Monday.
//        return DayNum(4 + (d - 4) / days * days);
//    }

    inline time_t toStartOfDayInterval(DayNum d, UInt64 days) const
    {
//        if (days == 1)
//            return toDate(d);
//        return lut[d / days * days].date;
        return toTimeTFromCCTZ(toCCTZ<cctz::civil_day>(d) + days);
    }

    inline time_t toStartOfHourInterval(time_t t, UInt64 hours) const
    {
//        if (hours == 1)
//            return toStartOfHour(t);
//        UInt64 seconds = hours * 3600;
//        t = t / seconds * seconds;
//        if (offset_is_whole_number_of_hours_everytime)
//            return t;
//        return toStartOfHour(t);
        auto datetime = toCCTZ<cctz::civil_hour>(t) + hours;
        datetime -= datetime.hour() % hours;
        return toTimeTFromCCTZ(datetime);
    }

    inline time_t toStartOfMinuteInterval(time_t t, UInt64 minutes) const
    {
//        if (minutes == 1)
//            return toStartOfMinute(t);
//        UInt64 seconds = 60 * minutes;
//        return t / seconds * seconds;
        auto datetime = toCCTZ<cctz::civil_minute>(t) + minutes;
        datetime -= datetime.minute() % minutes;
        return toTimeTFromCCTZ(datetime);
    }

    inline time_t toStartOfSecondInterval(time_t t, UInt64 seconds) const
    {
        if (seconds == 1)
            return t;
        return t / seconds * seconds;
    }

    /// Create DayNum from year, month, day of month.
    inline DayNum makeDayNum(UInt16 year, UInt8 month, UInt8 day_of_month) const
    {
        return toDayNumFromCCTZ(cctz::civil_day(year, month, day_of_month));
    }

    inline time_t makeDate(UInt16 year, UInt8 month, UInt8 day_of_month) const
    {
        return toTimeTFromCCTZ(cctz::civil_day(year, month, day_of_month));
    }

    /** Does not accept daylight saving time as argument: in case of ambiguity, it choose greater timestamp.
      */
    inline time_t makeDateTime(UInt16 year, UInt8 month, UInt8 day_of_month, UInt8 hour, UInt8 minute, UInt8 second) const
    {
        return toTimeTFromCCTZ(cctz::civil_second(year, month, day_of_month, hour, minute, second));
    }

    inline UInt32 toNumYYYYMM(time_t t) const
    {
        const auto day = toCCTZ<cctz::civil_day>(t);
        return day.year() * 100 + day.month();
    }

    inline UInt32 toNumYYYYMM(DayNum d) const
    {
        const auto day = toCCTZ<cctz::civil_day>(d);
        return day.year() * 100 + day.month();
    }

    inline UInt32 toNumYYYYMMDD(time_t t) const
    {
        const auto day = toCCTZ<cctz::civil_day>(t);
        return day.year() * 10000 + day.month() * 100 + day.day();
    }

    inline UInt32 toNumYYYYMMDD(DayNum d) const
    {
        const auto day = toCCTZ<cctz::civil_day>(d);
        return day.year() * 10000 + day.month() * 100 + day.day();
    }

    inline time_t YYYYMMDDToDate(UInt32 num) const
    {
        return makeDate(num / 10000, num / 100 % 100, num % 100);
    }

    inline DayNum YYYYMMDDToDayNum(UInt32 num) const
    {
        return makeDayNum(num / 10000, num / 100 % 100, num % 100);
    }


    inline UInt64 toNumYYYYMMDDhhmmss(time_t t) const
    {
        const auto val = toCCTZ<cctz::civil_second>(t);
        return val.second()
            + val.minute() * 100
            + val.hour() * 10000
            + UInt64(val.day()) * 1000000
            + UInt64(val.month()) * 100000000
            + UInt64(val.year()) * 10000000000;
    }

    inline time_t YYYYMMDDhhmmssToTime(UInt64 num) const
    {
        return makeDateTime(
            num / 10000000000,
            num / 100000000 % 100,
            num / 1000000 % 100,
            num / 10000 % 100,
            num / 100 % 100,
            num % 100);
    }

    /// Adding calendar intervals.
    /// Implementation specific behaviour when delta is too big.

    inline NO_SANITIZE_UNDEFINED time_t addDays(time_t t, Int64 delta) const
    {
        auto datetime = toCCTZ(t);
        return toTimeTFromCCTZ(cctz::civil_second(
                datetime.year(),
                datetime.month(),
                datetime.day() + delta,
                datetime.hour(),
                datetime.minute(),
                datetime.second()));
    }

    inline NO_SANITIZE_UNDEFINED time_t addWeeks(time_t t, Int64 delta) const
    {
        return addDays(t, delta * 7);
    }

//    inline UInt8 saturateDayOfMonth(UInt16 year, UInt8 month, UInt8 day_of_month) const
//    {
//        if (likely(day_of_month <= 28))
//            return day_of_month;

//        UInt8 days_in_month = daysInMonth(year, month);

//        if (day_of_month > days_in_month)
//            day_of_month = days_in_month;

//        return day_of_month;
//    }

//    /// If resulting month has less deys than source month, then saturation can happen.
//    /// Example: 31 Aug + 1 month = 30 Sep.
//    inline time_t addMonths(time_t t, Int64 delta) const
//    {
//        DayNum result_day = addMonths(toDayNum(t), delta);

//        time_t time_offset = toHour(t) * 3600 + toMinute(t) * 60 + toSecond(t);

//        if (time_offset >= lut[result_day].time_at_offset_change)
//            time_offset -= lut[result_day].amount_of_offset_change;

//        return lut[result_day].date + time_offset;
//    }

//    inline NO_SANITIZE_UNDEFINED DayNum addMonths(DayNum d, Int64 delta) const
//    {
//        const Values & values = lut[d];

//        Int64 month = static_cast<Int64>(values.month) + delta;

//        if (month > 0)
//        {
//            auto year = values.year + (month - 1) / 12;
//            month = ((month - 1) % 12) + 1;
//            auto day_of_month = saturateDayOfMonth(year, month, values.day_of_month);

//            return makeDayNum(year, month, day_of_month);
//        }
//        else
//        {
//            auto year = values.year - (12 - month) / 12;
//            month = 12 - (-month % 12);
//            auto day_of_month = saturateDayOfMonth(year, month, values.day_of_month);

//            return makeDayNum(year, month, day_of_month);
//        }
//    }

//    inline NO_SANITIZE_UNDEFINED time_t addQuarters(time_t t, Int64 delta) const
//    {
//        return addMonths(t, delta * 3);
//    }

//    inline NO_SANITIZE_UNDEFINED DayNum addQuarters(DayNum d, Int64 delta) const
//    {
//        return addMonths(d, delta * 3);
//    }

//    /// Saturation can occur if 29 Feb is mapped to non-leap year.
//    inline NO_SANITIZE_UNDEFINED time_t addYears(time_t t, Int64 delta) const
//    {
//        DayNum result_day = addYears(toDayNum(t), delta);

//        time_t time_offset = toHour(t) * 3600 + toMinute(t) * 60 + toSecond(t);

//        if (time_offset >= lut[result_day].time_at_offset_change)
//            time_offset -= lut[result_day].amount_of_offset_change;

//        return lut[result_day].date + time_offset;
//    }

//    inline NO_SANITIZE_UNDEFINED DayNum addYears(DayNum d, Int64 delta) const
//    {
//        const Values & values = lut[d];

//        auto year = values.year + delta;
//        auto month = values.month;
//        auto day_of_month = values.day_of_month;

//        /// Saturation to 28 Feb can happen.
//        if (unlikely(day_of_month == 29 && month == 2))
//            day_of_month = saturateDayOfMonth(year, month, day_of_month);

//        return makeDayNum(year, month, day_of_month);
//    }


    inline std::string timeToString(time_t t) const
    {
        const auto & values = toCCTZ(t);

        std::string s {"0000-00-00 00:00:00"};

        const auto year = values.year();
        s[0] += year / 1000;
        s[1] += (year / 100) % 10;
        s[2] += (year / 10) % 10;
        s[3] += year % 10;
        s[5] += values.month() / 10;
        s[6] += values.month() % 10;
        s[8] += values.day() / 10;
        s[9] += values.day() % 10;

        auto hour = values.hour();
        auto minute = values.minute();
        auto second = values.second();

        s[11] += hour / 10;
        s[12] += hour % 10;
        s[14] += minute / 10;
        s[15] += minute % 10;
        s[17] += second / 10;
        s[18] += second % 10;

        return s;
    }

    inline std::string dateToString(time_t t) const
    {
        const auto & values = toCCTZ(t);

        std::string s {"0000-00-00"};

        const auto year = values.year();
        s[0] += year / 1000;
        s[1] += (year / 100) % 10;
        s[2] += (year / 10) % 10;
        s[3] += year % 10;
        s[5] += values.month() / 10;
        s[6] += values.month() % 10;
        s[8] += values.day() / 10;
        s[9] += values.day() % 10;

        return s;
    }

    inline std::string dateToString(DayNum d) const
    {
        const auto & values = toCCTZ(d);

        std::string s {"0000-00-00"};

        const auto year = values.year();
        s[0] += year / 1000;
        s[1] += (year / 100) % 10;
        s[2] += (year / 10) % 10;
        s[3] += year % 10;
        s[5] += values.month() / 10;
        s[6] += values.month() % 10;
        s[8] += values.day() / 10;
        s[9] += values.day() % 10;

        return s;
    }
};

}

TEST(YYYYMMDDToDay, Test)
{
    std::cerr << YYYYMMDDHMMSSToSecond(19700101'00'00'00) << std::endl;
}

TEST(DateLUTTest, TimeValuesInMiddleOfRange)
{
    const DateLUTImpl lut("Europe/Minsk");
    const time_t time = 1568650811; // 2019-09-16 19:20:11 (Monday)

    EXPECT_EQ(lut.getTimeZone(), "Europe/Minsk");
//    EXPECT_EQ(lut.getOffsetAtStartOfEpoch(), 3600*3); // UTC-3

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
    const time_t time = 0; // 1970-01-01 00:00:00 (Thursday)

    EXPECT_EQ(lut.getTimeZone(), "UTC");

    EXPECT_EQ(lut.toDate(time), 0);
    EXPECT_EQ(lut.toMonth(time), 1);
    EXPECT_EQ(lut.toQuarter(time), 1);
    EXPECT_EQ(lut.toYear(time), 1970);
    EXPECT_EQ(lut.toDayOfMonth(time), 1);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), -259200 /*time_t*/); // 1969-12-29 00:00:00
//    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), ExtendedDayNum(-3) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 0 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(0) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 2678400 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), -2678400 /*time_t*/); // 1969-12-01 00:00:00
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
//    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), ExtendedDayNum(-3) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), -259200 /*time_t*/); // 1969-12-29 00:00:00
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

TEST(DateLUTTest, TimeValuesAtRightBoderOfRangeOfOLDLut)
{
    // Value is at the right border of the OLD (small) LUT, and provides meaningfull values where OLD LUT would provide garbage.
    const DateLUTImpl lut("UTC");

    const time_t time = 4294343873; // 2106-01-31T01:17:53 (Sunday)

    EXPECT_EQ(lut.getTimeZone(), "UTC");

    EXPECT_EQ(lut.toDate(time), 4294339200);
    EXPECT_EQ(lut.toMonth(time), 1);
    EXPECT_EQ(lut.toQuarter(time), 1);
    EXPECT_EQ(lut.toYear(time), 2106);
    EXPECT_EQ(lut.toDayOfMonth(time), 31);

    EXPECT_EQ(lut.toFirstDayOfWeek(time), 4293820800 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfWeek(time), DayNum(49697));
    EXPECT_EQ(lut.toFirstDayOfMonth(time), 4291747200 /*time_t*/); // 2016-01-01
    EXPECT_EQ(lut.toFirstDayNumOfMonth(time), DayNum(49673));
    EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), DayNum(49673) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfQuarter(time), 4291747200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayOfYear(time), 4291747200 /*time_t*/);
    EXPECT_EQ(lut.toFirstDayNumOfYear(time), DayNum(49673) /*DayNum*/);
    EXPECT_EQ(lut.toFirstDayOfNextMonth(time), 4294425600 /*time_t*/); // 2106-02-01
    EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), 4289068800 /*time_t*/); // 2105-12-01
    EXPECT_EQ(lut.daysInMonth(time), 31 /*UInt8*/);
    EXPECT_EQ(lut.toDateAndShift(time, 10), 4295203200 /*time_t*/); // 2106-02-10
    EXPECT_EQ(lut.toTime(time), 4673 /*time_t*/);
    EXPECT_EQ(lut.toHour(time), 1 /*unsigned*/);
    EXPECT_EQ(lut.toMinute(time), 17 /*unsigned*/);
    EXPECT_EQ(lut.toSecond(time), 53 /*unsigned*/);
    EXPECT_EQ(lut.toStartOfMinute(time), 4294343820 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFiveMinute(time), 4294343700 /*time_t*/);
    EXPECT_EQ(lut.toStartOfFifteenMinutes(time), 4294343700 /*time_t*/);
    EXPECT_EQ(lut.toStartOfTenMinutes(time), 4294343400 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHour(time), 4294342800 /*time_t*/);
    EXPECT_EQ(lut.toDayNum(time), DayNum(49703) /*DayNum*/);
    EXPECT_EQ(lut.toDayOfYear(time), 31 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeWeekNum(time), 7100 /*unsigned*/);
    EXPECT_EQ(lut.toISOYear(time), 2106 /*unsigned*/);
    EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), DayNum(49676) /*DayNum*/); // 2106-01-04
    EXPECT_EQ(lut.toFirstDayOfISOYear(time), 4292006400 /*time_t*/);
    EXPECT_EQ(lut.toISOWeek(time), 4 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeMonthNum(time), 25273 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeQuarterNum(time), 8424 /*unsigned*/);
    EXPECT_EQ(lut.toRelativeHourNum(time), 1192873 /*time_t*/);
    EXPECT_EQ(lut.toRelativeMinuteNum(time), 71572397 /*time_t*/);
    EXPECT_EQ(lut.toStartOfHourInterval(time, 5), 4294332000 /*time_t*/);
    EXPECT_EQ(lut.toStartOfMinuteInterval(time, 6), 4294343520 /*time_t*/);
    EXPECT_EQ(lut.toStartOfSecondInterval(time, 7), 4294343872 /*time_t*/);
    EXPECT_EQ(lut.toNumYYYYMM(time), 210601 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDD(time), 21060131 /*UInt32*/);
    EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), 21060131011753 /*UInt64*/);
    EXPECT_EQ(lut.addDays(time, 100), 4302983873 /*time_t*/);
    EXPECT_EQ(lut.addWeeks(time, 10), 4300391873 /*time_t*/);
    EXPECT_EQ(lut.addMonths(time, 10), 4320523073 /*time_t*/);                 // 2106-11-30 01:17:53
    EXPECT_EQ(lut.addQuarters(time, 10), 4373140673 /*time_t*/);               // 2108-07-31 01:17:53
    EXPECT_EQ(lut.addYears(time, 10), 4609876673 /*time_t*/);                  // 2116-01-31 01:17:53

    EXPECT_EQ(lut.timeToString(time), "2106-01-31 01:17:53" /*std::string*/);
    EXPECT_EQ(lut.dateToString(time), "2106-01-31" /*std::string*/);
}


class DateLUT_TimeZone : public ::testing::TestWithParam<const char * /* timezone name */>
{};

TEST_P(DateLUT_TimeZone, DISABLED_LoadAllTimeZones)
{
    // There are some assumptions and assertions about TZ data made in DateLUTImpl which are verified upon loading,
    // to make sure that those assertions are true for all timezones we are going to load all of them one by one.
    DateLUTImpl{GetParam()};
}

// Another long running test, shouldn't be run to often
TEST_P(DateLUT_TimeZone, VaidateTimeComponentsAroundEpoch)
{
    // Converting time around 1970-01-01 to hour-minute-seconds time components
    // could be problematic.
    const size_t max_failures_per_tz = 3;
    const auto timezone_name = GetParam();

    const auto * test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    const auto lut = DateLUTImpl(timezone_name);

    for (time_t i = -856147870; i < 86400 * 10000; i += 11 * 13 * 17 * 19)
    {
        SCOPED_TRACE(::testing::Message()
                << "\n\tTimezone: " << timezone_name
                << "\n\ttimestamp: " << i);
//                << "\n\t offset at start of epoch                  : " << lut.getOffsetAtStartOfEpoch()
//                << "\n\t offset_is_whole_number_of_hours_everytime : " << lut.getOffsetIsWholNumberOfHoursEveryWhere()
//                << "\n\t time_offset_epoch                         : " << lut.getTimeOffsetEpoch()
//                << "\n\t offset_at_start_of_lut                    : " << lut.getTimeOffsetAtStartOfLUT());

        EXPECT_GE(24, lut.toHour(i));
        EXPECT_GT(60, lut.toMinute(i));
        EXPECT_GT(60, lut.toSecond(i));

        const auto current_failures = countFailures(*test_info->result());
        if (current_failures.total > 0)
        {
            if (i < 0)
                i = -1;
        }

        if (current_failures.total >= max_failures_per_tz)
            break;
    }
}

TEST_P(DateLUT_TimeZone, getTimeZone)
{
    const auto & lut = DateLUT::instance(GetParam());

    EXPECT_EQ(GetParam(), lut.getTimeZone());
}

TEST_P(DateLUT_TimeZone, ZeroTime)
{
    const auto & lut = DateLUT::instance(GetParam());

    EXPECT_EQ(0, lut.toDayNum(time_t{0}));
    EXPECT_EQ(0, lut.toDayNum(DayNum{0}));
//    EXPECT_EQ(0, lut.toDayNum(ExtendedDayNum{0}));
}

// Group of tests for timezones that have or had some time ago an offset which is not multiple of 15 minutes.
INSTANTIATE_TEST_SUITE_P(ExoticTimezones,
    DateLUT_TimeZone,
    ::testing::ValuesIn(std::initializer_list<const char*>{
            "Africa/El_Aaiun",
            "Pacific/Apia",
            "Pacific/Enderbury",
            "Pacific/Fakaofo",
            "Pacific/Kiritimati",
    })
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimeZones,
    DateLUT_TimeZone,
    ::testing::ValuesIn(allTimezones())
);

std::ostream & operator<<(std::ostream & ostr, const DateLUTImpl::Values & v)
{
    return ostr << "DateLUTImpl::Values{"
            << "\n\t date              : " << v.date
            << "\n\t year              : " << static_cast<unsigned int>(v.year)
            << "\n\t month             : " << static_cast<unsigned int>(v.month)
            << "\n\t day               : " << static_cast<unsigned int>(v.day_of_month)
            << "\n\t weekday           : " << static_cast<unsigned int>(v.day_of_week)
            << "\n\t days in month     : " << static_cast<unsigned int>(v.days_in_month)
            << "\n\t offset change     : " << v.amount_of_offset_change
            << "\n\t offfset change at : " << v.time_at_offset_change
            << "\n}";
}

struct TimeRangeParam
{
    const cctz::civil_second begin;
    const cctz::civil_second end;
    const int step_in_seconds;
};

std::ostream & operator<<(std::ostream & ostr, const TimeRangeParam & param)
{
    const auto approximate_step = [](const int step) -> std::string
    {
        // Convert seconds to a string of seconds or fractional count of minutes/hours/days.
        static const size_t multipliers[] = {1 /*seconds to seconds*/, 60 /*seconds to minutes*/, 60 /*minutes to hours*/, 24 /*hours to days*/, 0 /*terminator*/};
        static const char* names[] = {"s", "m", "h", "d", nullptr};
        double result = step;
        size_t i = 0;
        for (; i < sizeof(multipliers)/sizeof(multipliers[0]) && result > multipliers[i]; ++i)
            result /= multipliers[i];

        char buffer[256] = {'\0'};
        std::snprintf(buffer, sizeof(buffer), "%.1f%s", result, names[i - 1]);
        return std::string{buffer};
    };

    return ostr << param.begin << " : " << param.end << " step: " << param.step_in_seconds << "s (" << approximate_step(param.step_in_seconds) << ")";
}

class DateLUT_Timezone_TimeRange : public ::testing::TestWithParam<std::tuple<const char* /*timezone_name*/, TimeRangeParam>>
{};

// refactored test from tests/date_lut3.cpp
TEST_P(DateLUT_Timezone_TimeRange, InRange)
{
    // for a time_t values in range [begin, end) to match with reference obtained from cctz:
    // compare date and time components: year, month, day, hours, minutes, seconds, formatted time string.
    const auto & [timezone_name, range_data] = GetParam();
    const auto & [begin, end, step] = range_data;

    const auto * test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    static const size_t max_failures_per_case = 3;
    cctz::time_zone tz;
    ASSERT_TRUE(cctz::load_time_zone(timezone_name, &tz));

    const auto & lut = DateLUT::instance(timezone_name);
    const auto start = cctz::convert(begin, tz).time_since_epoch().count();
    const auto stop = cctz::convert(end, tz).time_since_epoch().count();

    for (time_t expected_time_t = start; expected_time_t < stop; expected_time_t += step)
    {
        SCOPED_TRACE(expected_time_t);

        const auto tz_time = cctz::convert(std::chrono::system_clock::from_time_t(expected_time_t), tz);

        EXPECT_EQ(tz_time.year(), lut.toYear(expected_time_t));
        EXPECT_EQ(tz_time.month(), lut.toMonth(expected_time_t));
        EXPECT_EQ(tz_time.day(), lut.toDayOfMonth(expected_time_t));
        EXPECT_EQ(static_cast<int>(cctz::get_weekday(tz_time)) + 1, lut.toDayOfWeek(expected_time_t)); // tm.tm_wday Sunday is 0, while for DateLUTImpl it is 7
        EXPECT_EQ(cctz::get_yearday(tz_time), lut.toDayOfYear(expected_time_t));
        EXPECT_EQ(tz_time.hour(), lut.toHour(expected_time_t));
        EXPECT_EQ(tz_time.minute(), lut.toMinute(expected_time_t));
        EXPECT_EQ(tz_time.second(), lut.toSecond(expected_time_t));

        const auto time_string = cctz::format("%E4Y-%m-%d %H:%M:%S", std::chrono::system_clock::from_time_t(expected_time_t), tz);
        EXPECT_EQ(time_string, lut.timeToString(expected_time_t));

        // it makes sense to let test execute all checks above to simplify debugging,
        // but once we've found a bad apple, no need to dig deeper.
        if (countFailures(*test_info->result()).total >= max_failures_per_case)
            break;
    }
}

TEST_P(DateLUT_Timezone_TimeRange, TestFullRangeTimeZone)
{
    const auto & [timezone_name, range_data] = GetParam();
    const auto & [begin, end, step] = range_data;

    cctz::time_zone tz;
    ASSERT_TRUE(cctz::load_time_zone(timezone_name, &tz));

    const auto & lut = DateLUT::instance(timezone_name);
    const auto & full_tz = FullRangeTimeZone(tz);

    const auto start = cctz::convert(begin, tz).time_since_epoch().count();
    const auto stop = cctz::convert(end, tz).time_since_epoch().count();

    ASSERT_EQ(lut.getTimeZone(), full_tz.getTimeZone());

    for (time_t time = start; time < stop; time += step)
    {
        SCOPED_TRACE(::testing::Message("time: ") << time);

        EXPECT_EQ(lut.toDate(time), full_tz.toDate(time));
        EXPECT_EQ(lut.toMonth(time), full_tz.toMonth(time));
        EXPECT_EQ(lut.toQuarter(time), full_tz.toQuarter(time));
        EXPECT_EQ(lut.toYear(time), full_tz.toYear(time));
        EXPECT_EQ(lut.toDayOfMonth(time), full_tz.toDayOfMonth(time));

        EXPECT_EQ(lut.toFirstDayOfWeek(time), full_tz.toFirstDayOfWeek(time) /*time_t*/);
        EXPECT_EQ(lut.toFirstDayNumOfWeek(time), full_tz.toFirstDayNumOfWeek(time) /*DayNum*/);
        EXPECT_EQ(lut.toFirstDayOfMonth(time), full_tz.toFirstDayOfMonth(time) /*time_t*/);
        EXPECT_EQ(lut.toFirstDayNumOfMonth(time), full_tz.toFirstDayNumOfMonth(time) /*DayNum*/);
        EXPECT_EQ(lut.toFirstDayNumOfQuarter(time), full_tz.toFirstDayNumOfQuarter(time) /*DayNum*/);
        EXPECT_EQ(lut.toFirstDayOfQuarter(time), full_tz.toFirstDayOfQuarter(time) /*time_t*/);
        EXPECT_EQ(lut.toFirstDayOfYear(time), full_tz.toFirstDayOfYear(time) /*time_t*/);
        EXPECT_EQ(lut.toFirstDayNumOfYear(time), full_tz.toFirstDayNumOfYear(time) /*DayNum*/);
        EXPECT_EQ(lut.toFirstDayOfNextMonth(time), full_tz.toFirstDayOfNextMonth(time) /*time_t*/);
        EXPECT_EQ(lut.toFirstDayOfPrevMonth(time), full_tz.toFirstDayOfPrevMonth(time) /*time_t*/);
        EXPECT_EQ(static_cast<int>(lut.daysInMonth(time)), static_cast<int>(full_tz.daysInMonth(time)) /*UInt8*/);
        EXPECT_EQ(lut.toDateAndShift(time, 10), full_tz.toDateAndShift(time, 10) /*time_t*/);
//        EXPECT_EQ(lut.toTime(time), full_tz.toTime(time) /*time_t*/);
        EXPECT_EQ(lut.toHour(time), full_tz.toHour(time) /*unsigned*/);
        EXPECT_EQ(lut.toSecond(time), full_tz.toSecond(time) /*unsigned*/);
        EXPECT_EQ(lut.toMinute(time), full_tz.toMinute(time) /*unsigned*/);

        EXPECT_EQ(lut.toStartOfMinute(time), full_tz.toStartOfMinute(time) /*time_t*/);
        EXPECT_EQ(lut.toStartOfFiveMinute(time), full_tz.toStartOfFiveMinute(time) /*time_t*/);
        EXPECT_EQ(lut.toStartOfFifteenMinutes(time), full_tz.toStartOfFifteenMinutes(time) /*time_t*/);
        EXPECT_EQ(lut.toStartOfTenMinutes(time), full_tz.toStartOfTenMinutes(time) /*time_t*/);
        EXPECT_EQ(lut.toStartOfHour(time), full_tz.toStartOfHour(time) /*time_t*/);

        EXPECT_EQ(lut.toDayNum(time), full_tz.toDayNum(time) /*DayNum*/);
        EXPECT_EQ(lut.toDayOfYear(time), full_tz.toDayOfYear(time) /*unsigned*/);
        EXPECT_EQ(lut.toRelativeWeekNum(time), full_tz.toRelativeWeekNum(time) /*unsigned*/);
        EXPECT_EQ(lut.toISOYear(time), full_tz.toISOYear(time) /*unsigned*/);
//        EXPECT_EQ(lut.toFirstDayNumOfISOYear(time), full_tz.toFirstDayNumOfISOYear(time) /*DayNum*/);
//        EXPECT_EQ(lut.toFirstDayOfISOYear(time), full_tz.toFirstDayOfISOYear(time) /*time_t*/);
//        EXPECT_EQ(lut.toISOWeek(time), full_tz.toISOWeek(time) /*unsigned*/);
        EXPECT_EQ(lut.toRelativeMonthNum(time), full_tz.toRelativeMonthNum(time) /*unsigned*/);
        EXPECT_EQ(lut.toRelativeQuarterNum(time), full_tz.toRelativeQuarterNum(time) /*unsigned*/);
        EXPECT_EQ(lut.toRelativeHourNum(time), full_tz.toRelativeHourNum(time) /*time_t*/);
        EXPECT_EQ(lut.toRelativeMinuteNum(time), full_tz.toRelativeMinuteNum(time) /*time_t*/);

        const auto dn = lut.toDayNum(time);
        for (const auto shift : {1, 3, 7, 9, 11})
        {
            SCOPED_TRACE(::testing::Message("shift: ") << shift);
            EXPECT_EQ(lut.toStartOfDayInterval(dn, shift), full_tz.toStartOfDayInterval(dn, shift));
            EXPECT_EQ(lut.toStartOfMonthInterval(dn, shift), full_tz.toStartOfMonthInterval(dn, shift));
            EXPECT_EQ(lut.toStartOfHourInterval(time, shift), full_tz.toStartOfHourInterval(time, shift) /*time_t*/);
            EXPECT_EQ(lut.toStartOfMinuteInterval(time, shift), full_tz.toStartOfMinuteInterval(time, shift) /*time_t*/);
            EXPECT_EQ(lut.toStartOfSecondInterval(time, shift), full_tz.toStartOfSecondInterval(time, shift) /*time_t*/);

            if (HasFailure())
                break;
        }

        EXPECT_EQ(lut.toNumYYYYMM(time), full_tz.toNumYYYYMM(time) /*UInt32*/);
        EXPECT_EQ(lut.toNumYYYYMMDD(time), full_tz.toNumYYYYMMDD(time) /*UInt32*/);
        EXPECT_EQ(lut.toNumYYYYMMDDhhmmss(time), full_tz.toNumYYYYMMDDhhmmss(time) /*UInt64*/);

        EXPECT_EQ(lut.addDays(time, 100), full_tz.addDays(time, 100) /*time_t*/);
        EXPECT_EQ(lut.addWeeks(time, 100), full_tz.addWeeks(time, 100) /*time_t*/);
//        EXPECT_EQ(lut.addMonths(time, 100), full_tz.addMonths(time, 100) /*time_t*/);
//        EXPECT_EQ(lut.addQuarters(time, 100), full_tz.addQuarters(time, 100) /*time_t*/);
//        EXPECT_EQ(lut.addYears(time, 10), full_tz.addYears(time, 10) /*time_t*/);
        EXPECT_EQ(lut.timeToString(time), full_tz.timeToString(time) /*std::string*/);
        EXPECT_EQ(lut.dateToString(time), full_tz.dateToString(time) /*std::string*/);

        if (HasFailure())
            break;
    }
}

/** Next tests are disabled due to following reasons:
 *  1. They are huge and take enormous amount of time to run
 *  2. Current implementation of DateLUTImpl is inprecise and some cases fail and it seems impractical to try to fix those.
 *  3. Many failures (~300) were fixed while refactoring, about ~40 remain the same and 3 new introduced:
 *      "Asia/Gaza"
 *      "Pacific/Enderbury"
 *      "Pacific/Kiritimati"
 *  So it would be tricky to skip knonw failures to allow all unit tests to pass.
 */
INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_Year2010,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            // Values from tests/date_lut3.cpp
            {YYYYMMDDToDay(20101031), YYYYMMDDToDay(20101101), 15 * 60},
            {YYYYMMDDToDay(20100328), YYYYMMDDToDay(20100330), 15 * 60}
        }))
);

INSTANTIATE_TEST_SUITE_P(Year2010,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::Values("Europe/Minsk", "UTC"),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            // Values from tests/date_lut3.cpp
            {YYYYMMDDToDay(20101031), YYYYMMDDToDay(20101101), 15 * 60},
            {YYYYMMDDToDay(20100328), YYYYMMDDToDay(20100330), 15 * 60}
        }))
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_Year1970_WHOLE,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            // Values from tests/date_lut3.cpp
            {YYYYMMDDToDay(19700101), YYYYMMDDToDay(19701231), 3191 /*53m 11s*/},
        }))
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_Year2010_WHOLE,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            // Values from tests/date_lut3.cpp
            {YYYYMMDDToDay(20100101), YYYYMMDDToDay(20101231), 3191 /*53m 11s*/},
        }))
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_Year2020_WHOLE,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            // Values from tests/date_lut3.cpp
            {YYYYMMDDToDay(20200101), YYYYMMDDToDay(20201231), 3191 /*53m 11s*/},
        }))
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_PreEpoch,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            {YYYYMMDDToDay(19500101), YYYYMMDDToDay(19600101), 15 * 60},
            {YYYYMMDDToDay(19300101), YYYYMMDDToDay(19350101), 11 * 15 * 60}
        }))
);

INSTANTIATE_TEST_SUITE_P(DISABLED_AllTimezones_Year1970,
    DateLUT_Timezone_TimeRange,
    ::testing::Combine(
        ::testing::ValuesIn(allTimezones()),
        ::testing::ValuesIn(std::initializer_list<TimeRangeParam>{
            {YYYYMMDDToDay(19700101), YYYYMMDDToDay(19700201), 15 * 60},
            {YYYYMMDDToDay(19700101), YYYYMMDDToDay(19701231), 11 * 13 * 17}
//            // 11 was chosen as a number which can't divide product of 2-combinarions of (7, 24, 60),
//            // to reduce likelehood of hitting same hour/minute/second values for different days.
//            // + 12 is just to make sure that last day is covered fully.
//            {0, 0 + 11 * 3600 * 24 + 12, 11},
        }))
);
