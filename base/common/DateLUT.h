#pragma once

#include "DateLUTImpl.h"
#include "TimeZone.h"

#include "defines.h"

#include <boost/noncopyable.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>


/// This class provides lazy initialization and lookup of singleton DateLUTImpl objects for a given timezone.
class DateLUT : private boost::noncopyable
{
public:
    /// Return singleton DateLUTImpl instance for the default time zone.
    static ALWAYS_INLINE const DateLUTImpl & instance()
    {
        const auto & date_lut = getInstance();
        return date_lut.default_timezone.load(std::memory_order_acquire)->getDefaultLUT();
    }

    /// Return singleton DateLUTImpl instance for a given time zone.
    static ALWAYS_INLINE const DateLUTImpl & instance(const std::string & time_zone)
    {
        const auto & date_lut = getInstance();
        if (time_zone.empty())
            return date_lut.default_timezone.load(std::memory_order_acquire)->getDefaultLUT();

        return date_lut.getImplementation(time_zone).getDefaultLUT();
    }

    /// Return singleton DateLUTImpl instance for a given time zone.
    static ALWAYS_INLINE const DateLUTImpl & getLUT(const std::string & time_zone)
    {
        return DateLUT::instance(time_zone);
    }

    /// Return singleton TimeZoneImpl instance for a given time zone.
    static ALWAYS_INLINE const TimeZoneImpl & getTimeZone(const std::string & time_zone)
    {
        const auto & date_lut = getInstance();
        if (time_zone.empty())
            return *date_lut.default_timezone.load(std::memory_order_acquire);

        return date_lut.getImplementation(time_zone);
    }

    static ALWAYS_INLINE const TimeZoneImpl & getTimeZone()
    {
        const auto & date_lut = getInstance();
        return *date_lut.default_timezone.load(std::memory_order_acquire);
    }

    static void setDefaultTimezone(const std::string & time_zone)
    {
        auto & date_lut = getInstance();
        const auto & impl = date_lut.getImplementation(time_zone);
        date_lut.default_timezone.store(&impl, std::memory_order_release);
    }

protected:
    DateLUT();
    ~DateLUT();

private:
    static DateLUT & getInstance();

    const TimeZoneImpl & getImplementation(const std::string & time_zone) const;

    using TimeZoneImplPtr = std::unique_ptr<TimeZoneImpl>;

    /// Time zone name -> implementation.
    mutable std::unordered_map<std::string, TimeZoneImplPtr> timezones;
    mutable std::mutex mutex;

    std::atomic<const TimeZoneImpl *> default_timezone;
};
