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
//    /// Return singleton DateLUTImpl instance for the default time zone.
//    static ALWAYS_INLINE const DateLUTImpl & instance()
//    {
//        const auto & date_lut = getInstance();
//        return date_lut.default_timezone.load(std::memory_order_acquire)->getDefaultLUT();
//    }

//    /// Return singleton DateLUTImpl instance for a given time zone.
//    static ALWAYS_INLINE const DateLUTImpl & instance(const std::string & time_zone)
//    {
//        const auto & date_lut = getInstance();
//        if (time_zone.empty())
//            return date_lut.default_timezone.load(std::memory_order_acquire)->getDefaultLUT();

//        return date_lut.getImplementation(time_zone).getDefaultLUT();
//    }

////    /// Return singleton DateLUTImpl instance for a given time zone.
////    static ALWAYS_INLINE const DateLUTImpl & getLUT(const std::string & time_zone)
////    {
////        return DateLUT::getTimeZone(time_zone);
////    }

    /// Return singleton TimeZone instance for a given time zone.
    static ALWAYS_INLINE const TimeZone & getTimeZone(const std::string & time_zone)
    {
        const auto & storage = getInstance();
        if (time_zone.empty())
            return *storage.default_timezone.load(std::memory_order_acquire);

        return storage.getImplementation(time_zone);
    }

    static ALWAYS_INLINE const TimeZone & getTimeZone()
    {
        const auto & storage = getInstance();
        return *storage.default_timezone.load(std::memory_order_acquire);
    }

    static void setDefaultTimezone(const std::string & time_zone)
    {
        auto & storage = getInstance();
        const auto & impl = storage.getImplementation(time_zone);
        storage.default_timezone.store(&impl, std::memory_order_release);
    }

protected:
    DateLUT();
    ~DateLUT();

private:
    static DateLUT & getInstance();

    const TimeZone & getImplementation(const std::string & time_zone) const;

    using TimeZoneImplPtr = std::unique_ptr<TimeZone>;

    /// Time zone name -> implementation.
    mutable std::unordered_map<std::string, TimeZoneImplPtr> timezones;
    mutable std::mutex mutex;

    // Not owned here.
    std::atomic<const TimeZone *> default_timezone;
};
