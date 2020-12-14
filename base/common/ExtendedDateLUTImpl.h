#pragma once

#include "DateLUTImpl.h"

#include <string>
#include <atomic>

// how many seconds (roughly) are covered by LUT.
#define DATE_LUT_SIZE_IN_SECONDS DATE_LUT_MAX_DAY_NUM * 3600ULL * 24ULL


/** ExtendedDateLUTImpl allows for various date/time related operations in a given timezone.
 *
 * Basically a wrapper around DateLUTImpl, provides the (almost) same interface and extends
 * supported range beyound 1970-2015 year span.
 *
 * Exact range depends on TimeZone::luts_size, for value of 5 that is roughly from 1698 to 2378.
 *
 * Contains several DateLUTImpl and chooses the one to operate on based on time_t, DayNum or YYYY/MM/DD value.
 *
 * Currently known limitations:
 *  - Bogus values outside of range
 *  - Fails to detect if result of addDays(), addYears(), etc. crosses LUT boundary,
 *    hence it will produce bogus results around the edges of LUT with large enough diffs.
 *  - Finds (and loads if required) proper LUT at run-time, hence performance of almost every
 *    method is worse than corresponding method of DateLUTImpl, athough diffence shouldn't be really big.
 */
class ExtendedDateLUTImpl
{
public:
    explicit ExtendedDateLUTImpl(const DateLUTImpl & timezone_lut);
    ~ExtendedDateLUTImpl();

public:
    using RelativeNum = DateLUTImpl::RelativeNum;
    using Values = DateLUTImpl::Values;
    static const Int64 lut_size_in_seconds = DATE_LUT_SIZE_IN_SECONDS;

    static inline Int32 getLUTIndex(time_t time)
    {
        // Split into buckets of exactly DATE_LUT_SIZE_IN_SECONDS items
        const Int64 r = time % lut_size_in_seconds;
        const Int64 index = time / lut_size_in_seconds - (r < 0);
        return static_cast<Int32>(index);
    }

    inline Int32 getLUTIndex(GlobalDayNum day) const
    {
        auto p = std::upper_bound(std::begin(luts), std::end(luts), day,
                         [](const GlobalDayNum & d, const LutEntry & entry)
        {
           return d < entry.min_daynum;
        });
        // if the given time is not in the pre-calculated range of the LUTS, jsut give the last LUT.
        return getLUTIndex((--p)->min_time);
    }

    inline Int32 getLUTIndex(Int16 year, UInt8 month, UInt8 day) const
    {
        const Int32 yyyymmdd = year * 10000 + month * 100 + day;
        auto p = std::upper_bound(std::begin(luts), std::end(luts), yyyymmdd,
                         [](const Int32 & value, const LutEntry & entry)
        {
           return value < entry.min_yyyymmdd;
        });
        // if the given time is not in the pre-calculated range of the LUTS, jsut give the last LUT.
        return getLUTIndex((--p)->min_time);
    }

    const DateLUTImpl & getLUTByIndexMaybeWithLock(Int32 lut_index) const;

    inline const DateLUTImpl & getLUTByIndex(Int32 lut_index) const
    {
        if (lut_index == 0)
            return default_lut;

        return getLUTByIndexMaybeWithLock(lut_index);
    }

    template <typename ... Args>
    inline const DateLUTImpl & getLUT(Args && ...args) const
    {
        return getLUTByIndex(getLUTIndex(std::forward<Args>(args)...));
    }

//    template <typename T>
//    static inline auto convertArg(const DateLUTImpl & /*lut*/, T t) { return t; }
    static inline auto convertArg(const DateLUTImpl & lut, GlobalDayNum d) { return lut.toLUTDayNum(d); }
    static inline auto convertArg(const DateLUTImpl & /*lut*/, time_t t) { return t; }

    template <typename T>
    static inline auto convertResult(const DateLUTImpl & /*lut*/, T && u) { return u; } // Generic values, like year, week number, etc that do not require convertion.
    static inline auto convertResult(const DateLUTImpl & /*lut*/, time_t t) { return t; }
    static inline auto convertResult(const DateLUTImpl & lut, DayNum d) { return lut.toGlobalDayNum(d); }

    template <typename T>
    using convertArg_t = decltype(convertArg(std::declval<DateLUTImpl>(), std::declval<std::decay_t<T>>()));

    // Call specific const member function of DateLUTImpl,
    // converting first argument from "global" DayNum to LUT-specific value (with convertArg),
    // and converting result back from LUT-specific values to "global" DayNum (with convertResult).
    // NOTE: it might be possibe to get rid of explicit 'ResultType' template argument,
    // but that requires too much dark magic, so let it be as it is for now.
    template <typename ResultType, typename FirstArgType, typename ...Args>
    inline auto callAndConvert(ResultType (DateLUTImpl::*f)(convertArg_t<FirstArgType>, Args ...) const,
                        FirstArgType first_arg, Args ...args) const
    {
        const auto & lut = getLUT(first_arg);
        const auto & first_arg_converted = convertArg(lut, first_arg);

        const ResultType & result = (lut.*f)(first_arg_converted, std::forward<Args>(args)...);
        auto result_converted = convertResult(lut, result);

        return result_converted;
    }

    // TODO: should we allow negative values? If yes, that should produce negative year, but other parts should be positive.
    inline static std::tuple<Int16 /*year*/, UInt8 /*month*/, UInt8 /*day_of_month*/>
    splitYYYYMMDD(UInt64 num)
    {
        return {num / 10000, num / 100 % 100, num % 100};
    }

public:
    // For some reason, these do not work, hence there are not private sections in this class.
//    friend class TimeZoneTest;
//    FRIEND_TEST(TimeZoneTest, getLUTIndex);
//    friend class TimeZoneRangeTest;
//    FRIEND_TEST(TimeZoneRangeTest, UTC);

    // If you want more performance (by avoiding finding proper LUT) and know that your values fall in default_lut range (1970-2105).
    inline const DateLUTImpl & getDefaultLUT() const
    {
        return default_lut;
    }

    const std::string & getTimeZone() const
    {
        return default_lut.getTimeZone();
    }

    enum class RangeType {TIME_T, DAY_NUM, YYYYMMDD};
    template <RangeType R>
    auto getRange() const
    {
        const auto & first = luts[0];
        const auto & last = luts[luts_size];
        if constexpr (R == RangeType::TIME_T)
        {
            return std::make_tuple(first.min_time, last.min_time);
        }
        else if constexpr (R == RangeType::DAY_NUM)
        {
            return std::make_tuple(first.min_daynum, last.min_daynum);
        }
        else if constexpr (R == RangeType::YYYYMMDD)
        {
            return std::make_tuple(first.min_yyyymmdd, last.min_yyyymmdd);
        }
    }


    // Methods similar to DateLUTImpl
    inline time_t toDate(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toDate, t);
    }
    inline time_t toDate(GlobalDayNum d) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toDate, d);
    }

    inline unsigned toMonth(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toMonth, t);
    }
    inline unsigned toQuarter(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toQuarter, t);
    }
    inline Int16 toYear(time_t t) const
    {
        return callAndConvert<Int16>(&DateLUTImpl::toYear, t);
    }
    inline unsigned toDayOfWeek(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfWeek, t);
    }
    inline unsigned toDayOfMonth(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfMonth, t);
    }
    inline UInt8 daysInMonth(GlobalDayNum d) const
    {
        return callAndConvert<UInt8>(&DateLUTImpl::daysInMonth, d);
    }
    inline UInt8 daysInMonth(time_t t) const
    {
        return callAndConvert<UInt8>(&DateLUTImpl::daysInMonth, t);
    }
    inline UInt8 daysInMonth(Int16 year, UInt8 month) const
    {
        return getLUT(year, month, 0).daysInMonth(year, month);
    }
    inline time_t toDateAndShift(time_t t, Int32 days) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toDateAndShift, t, days);
    }
    inline time_t toTime(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toTime, t);
    }
    inline unsigned toHour(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toHour, t);
    }
    inline unsigned toMinute(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toMinute, t);
    }

    // =============================================================================================
    // NOTE: All those methdods do not depend on LUT, can be done locally against `default_lut`
    // {
    inline unsigned toSecond(time_t t) const
    {
        return default_lut.toSecond(t);
    }
    inline time_t toStartOfMinute(time_t t) const
    {
        return default_lut.toStartOfMinute(t);
    }
    inline time_t toStartOfFiveMinute(time_t t) const
    {
        return default_lut.toStartOfFiveMinute(t);
    }
    inline time_t toStartOfFifteenMinutes(time_t t) const
    {
        return default_lut.toStartOfFifteenMinutes(t);
    }
    inline time_t toStartOfTenMinutes(time_t t) const
    {
        return default_lut.toStartOfTenMinutes(t);
    }
    // }
    // =============================================================================================

    inline time_t toStartOfHour(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toStartOfHour, t);
    }
    inline GlobalDayNum toDayNum(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toDayNum, t);
    }
    inline time_t fromDayNum(GlobalDayNum d) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::fromDayNum, d);
    }
    inline unsigned toMonth(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toMonth, d);
    }
    inline unsigned toQuarter(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toQuarter, d);
    }
    inline Int16 toYear(GlobalDayNum d) const
    {
        return callAndConvert<Int16>(&DateLUTImpl::toYear, d);
    }
    inline unsigned toDayOfWeek(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfWeek, d);
    }
    inline unsigned toDayOfMonth(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfMonth, d);
    }
    inline unsigned toDayOfYear(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfYear, d);
    }
    inline unsigned toDayOfYear(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toDayOfYear, t);
    }
    inline RelativeNum toRelativeWeekNum(GlobalDayNum d) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toRelativeWeekNum, d);
    }
    inline RelativeNum toRelativeWeekNum(time_t t) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toRelativeWeekNum, t);
    }
    inline unsigned toISOYear(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toISOYear, d);
    }
    inline unsigned toISOYear(time_t t) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toISOYear, t);
    }
    inline GlobalDayNum toFirstDayNumOfISOYear(GlobalDayNum d) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfISOYear, d);
    }
    inline GlobalDayNum toFirstDayNumOfISOYear(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfISOYear, t);
    }
    inline time_t toFirstDayOfISOYear(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfISOYear, t);
    }
    inline RelativeNum toISOWeek(GlobalDayNum d) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toISOWeek, d);
    }
    inline RelativeNum toISOWeek(time_t t) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toISOWeek, t);
    }
    inline YearWeek toYearWeek(GlobalDayNum d, UInt8 week_mode) const
    {
        return callAndConvert<YearWeek>(&DateLUTImpl::toYearWeek, d, week_mode);
    }
    inline YearWeek toYearWeekOfNewyearMode(GlobalDayNum d, bool monday_first_mode) const
    {
        return callAndConvert<YearWeek>(&DateLUTImpl::toYearWeekOfNewyearMode, d, monday_first_mode);
    }
    inline GlobalDayNum toFirstDayNumOfWeek(GlobalDayNum d, UInt8 week_mode) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfWeek, d, week_mode);
    }
    inline UInt8 check_week_mode(UInt8 mode) const
    {
        return default_lut.check_week_mode(mode);
    }
    inline unsigned calc_weekday(GlobalDayNum d, bool sunday_first_day_of_week) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::calc_weekday, d, sunday_first_day_of_week);
    }
    inline unsigned calc_days_in_year(Int16 year) const
    {
        return default_lut.calc_days_in_year(year);
    }
    inline unsigned toRelativeMonthNum(GlobalDayNum d) const
    {
        return callAndConvert<unsigned>(&DateLUTImpl::toRelativeMonthNum, d);
    }
    inline RelativeNum toRelativeMonthNum(time_t t) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toRelativeMonthNum, t);
    }
    inline RelativeNum toRelativeQuarterNum(GlobalDayNum d) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toRelativeQuarterNum, d);
    }
    inline RelativeNum toRelativeQuarterNum(time_t t) const
    {
        return callAndConvert<RelativeNum>(&DateLUTImpl::toRelativeQuarterNum, t);
    }
    inline time_t toRelativeHourNum(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toRelativeHourNum, t);
    }
    inline time_t toRelativeHourNum(GlobalDayNum d) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toRelativeHourNum, d);
    }
    inline time_t toRelativeMinuteNum(time_t t) const
    {
        return default_lut.toRelativeMinuteNum(t);
    }
    inline time_t toRelativeMinuteNum(GlobalDayNum d) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toRelativeMinuteNum, d);
    }
    inline GlobalDayNum makeDayNum(Int16 year, UInt8 month, UInt8 day_of_month) const
    {
        const auto & lut = getLUT(year, month, day_of_month);
        return convertResult(lut, lut.makeDayNum(year, month, day_of_month));
    }
    inline time_t makeDate(Int16 year, UInt8 month, UInt8 day_of_month) const
    {
        const auto & lut = getLUT(year, month, day_of_month);
        return convertResult(lut, lut.makeDate(year, month, day_of_month));
    }
    inline time_t makeDateTime(Int16 year, UInt8 month, UInt8 day_of_month, UInt8 hour, UInt8 minute, UInt8 second) const
    {
        const auto & lut = getLUT(year, month, day_of_month);
        return lut.makeDateTime(year, month, day_of_month, hour, minute, second);
    }
    inline const Values & getValues(GlobalDayNum d) const
    {
        const auto & lut = getLUT(d);
        return lut.getValues(convertArg(lut, d));
    }
    inline const Values & getValues(time_t t) const
    {
        const auto & lut = getLUT(t);
        return lut.getValues(convertArg(lut, t));
    }
    inline UInt32 toNumYYYYMM(time_t t) const
    {
        return callAndConvert<UInt32>(&DateLUTImpl::toNumYYYYMM, t);
    }
    inline UInt32 toNumYYYYMM(GlobalDayNum d) const
    {
        return callAndConvert<UInt32>(&DateLUTImpl::toNumYYYYMM, d);
    }
    inline UInt32 toNumYYYYMMDD(time_t t) const
    {
        return callAndConvert<UInt32>(&DateLUTImpl::toNumYYYYMMDD, t);
    }
    inline UInt32 toNumYYYYMMDD(GlobalDayNum d) const
    {
        return callAndConvert<UInt32>(&DateLUTImpl::toNumYYYYMMDD, d);
    }

    inline time_t YYYYMMDDToDate(UInt32 num) const
    {
        const auto [year, month, day_of_month] = splitYYYYMMDD(num);
        const auto & lut = getLUT(year, month, day_of_month);

        return convertResult(lut, lut.YYYYMMDDToDate(num));
    }
    inline GlobalDayNum YYYYMMDDToDayNum(UInt32 num) const
    {
        const auto [year, month, day_of_month] = splitYYYYMMDD(num);
        const auto & lut = getLUT(year, month, day_of_month);

        return convertResult(lut, lut.YYYYMMDDToDayNum(num));
    }
    inline UInt64 toNumYYYYMMDDhhmmss(time_t t) const
    {
        return callAndConvert<UInt64>(&DateLUTImpl::toNumYYYYMMDDhhmmss, t);
    }
    inline time_t YYYYMMDDhhmmssToTime(UInt64 num) const
    {
        const auto [year, month, day_of_month] = splitYYYYMMDD(num / 100'00'00);
        const auto & lut = getLUT(year, month, day_of_month);

        return convertResult(lut, lut.YYYYMMDDhhmmssToTime(num));
    }

// =============================================================================================
// TODO: handle crossing the LUT border
// {
    inline time_t addDays(time_t t, Int64 delta) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::addDays, t, delta);
    }
    inline time_t addWeeks(time_t t, Int64 delta) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::addWeeks, t, delta);
    }
    inline time_t addMonths(time_t t, Int64 delta) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::addMonths, t, delta);
    }
    inline GlobalDayNum addMonths(GlobalDayNum d, Int64 delta) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::addMonths, d, delta);
    }
    inline time_t addQuarters(time_t t, Int64 delta) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::addQuarters, t, delta);
    }
    inline GlobalDayNum addQuarters(GlobalDayNum d, Int64 delta) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::addQuarters, d, delta);
    }
    inline time_t addYears(time_t t, Int64 delta) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::addYears, t, delta);
    }
    inline GlobalDayNum addYears(GlobalDayNum d, Int64 delta) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::addYears, d, delta);
    }

    inline GlobalDayNum toStartOfYearInterval(GlobalDayNum d, UInt64 years) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toStartOfYearInterval, d, years);
    }
    inline GlobalDayNum toStartOfQuarterInterval(GlobalDayNum d, UInt64 quarters) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toStartOfQuarterInterval, d, quarters);
    }
    inline GlobalDayNum toStartOfMonthInterval(GlobalDayNum d, UInt64 months) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toStartOfMonthInterval, d, months);
    }
    inline GlobalDayNum toStartOfWeekInterval(GlobalDayNum d, UInt64 weeks) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toStartOfWeekInterval, d, weeks);
    }
    inline time_t toStartOfDayInterval(GlobalDayNum d, UInt64 days) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toStartOfDayInterval, d, days);
    }
    inline time_t toStartOfHourInterval(time_t t, UInt64 hours) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toStartOfHourInterval, t, hours);
    }
    inline time_t toStartOfMinuteInterval(time_t t, UInt64 minutes) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toStartOfMinuteInterval, t, minutes);
    }
    inline time_t toStartOfSecondInterval(time_t t, UInt64 seconds) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toStartOfSecondInterval, t, seconds);
    }

    inline time_t toFirstDayOfWeek(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfWeek, t);
    }
    inline GlobalDayNum toFirstDayNumOfWeek(GlobalDayNum d) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfWeek, d);
    }

    inline GlobalDayNum toFirstDayNumOfWeek(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfWeek, t);
    }
    inline time_t toFirstDayOfMonth(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfMonth, t);
    }
    inline GlobalDayNum toFirstDayNumOfMonth(GlobalDayNum d) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfMonth, d);
    }
    inline GlobalDayNum toFirstDayNumOfMonth(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfMonth, t);
    }
    inline GlobalDayNum toFirstDayNumOfQuarter(GlobalDayNum d) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfQuarter, d);
    }
    inline GlobalDayNum toFirstDayNumOfQuarter(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfQuarter, t);
    }
    inline time_t toFirstDayOfQuarter(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfQuarter, t);
    }
    inline time_t toFirstDayOfYear(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfYear, t);
    }
    inline GlobalDayNum toFirstDayNumOfYear(GlobalDayNum d) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfYear, d);
    }
    inline GlobalDayNum toFirstDayNumOfYear(time_t t) const
    {
        return callAndConvert<DayNum>(&DateLUTImpl::toFirstDayNumOfYear, t);
    }
    inline time_t toFirstDayOfNextMonth(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfNextMonth, t);
    }
    inline time_t toFirstDayOfPrevMonth(time_t t) const
    {
        return callAndConvert<time_t>(&DateLUTImpl::toFirstDayOfPrevMonth, t);
    }
// }
// =============================================================================================

    inline UInt8 saturateDayOfMonth(Int16 year, UInt8 month, UInt8 day_of_month) const
    {
        const auto & lut = getLUT(year, month, day_of_month);
        return convertResult(lut, lut.saturateDayOfMonth(year, month, day_of_month));
    }

    inline std::string timeToString(time_t t) const { return getLUT(t).timeToString(t); }
    inline std::string dateToString(time_t t) const { return getLUT(t).dateToString(t); }
    inline std::string dateToString(GlobalDayNum d) const { return getLUT(d).dateToString(DayNum(d.toUnderType())); }

private:
    const DateLUTImpl & default_lut;

    struct LutEntry
    {
        time_t min_time = 0;
        GlobalDayNum min_daynum = GlobalDayNum{0};
        Int32 min_yyyymmdd = 0;
        mutable std::atomic<const DateLUTImpl*> lut = nullptr;
    };

    static const size_t luts_size = 5;
    static_assert(luts_size % 2 == 1, "luts_size should be odd");

    // This is to simplify finding LUT index by DayNum and year/month/day, must be sorted for binary search to work.
    LutEntry luts[luts_size + 1]; // +1 since last is a dummy value used for determinig TimeZone range.
};

