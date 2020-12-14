#include <common/TimeZone.h>

#if __has_include(<cctz/civil_time.h>)
#include <cctz/civil_time.h> // bundled, debian
#else
#include <civil_time.h> // freebsd
#endif

#if __has_include(<cctz/time_zone.h>)
#include <cctz/time_zone.h>
#else
#include <time_zone.h>
#endif

#include <cassert>
#include <memory>


ExtendedDateLUTImpl::ExtendedDateLUTImpl(const DateLUTImpl &timezone_lut)
      : default_lut(timezone_lut)
{
    luts[luts_size/2].lut.store(&default_lut, std::memory_order_relaxed);

    // This is a hack, but it works since cctz::time_zone is just a wrapper around pointer owned globally.
    // Hence it is safe to pass it by value, and getCCTZ() doesn't result in reading TZ from disk
    // when called second time.
    const auto & tz = default_lut.getCCTZ();
    const cctz::civil_day epoch{1970, 1, 1};

    for (size_t i = 0; i < sizeof(luts)/sizeof(luts[0]); ++i)
    {
        const auto lut_index_value = i - static_cast<int>(luts_size) / 2;
        const time_t time = lut_size_in_seconds * lut_index_value;

        auto & entry = luts[i];
        cctz::time_zone::absolute_lookup lut_starting_point = tz.lookup(std::chrono::system_clock::from_time_t(time));

        entry.min_time = time;
        entry.min_daynum = cctz::civil_day(lut_starting_point.cs) - epoch;
        entry.min_yyyymmdd = lut_starting_point.cs.year() * 10000 + lut_starting_point.cs.month() * 100 + lut_starting_point.cs.day();

        if (time != 0)
        {
            assert(entry.min_daynum != 0);
        }
    }
}

ExtendedDateLUTImpl::~ExtendedDateLUTImpl()
{
    for (size_t i = 0; i < sizeof(luts)/sizeof(luts[0]); ++i)
    {
        // LUT-0, not owned by TimeZone via luts
        if (i - luts_size / 2 == 0)
            continue;

        delete luts[i].lut.exchange(nullptr, std::memory_order_consume);
    }
}


const DateLUTImpl & ExtendedDateLUTImpl::getLUTByIndexMaybeWithLock(Int32 lut_index) const
{
    const auto lut_array_index = lut_index + static_cast<Int32>(luts_size) / 2;
    if (lut_array_index < 0 || lut_array_index >= static_cast<Int32>(luts_size))
        // since index is out of bounds, we are free to provide bogus values,
        // so lets reuse default_lut because it is always present.
        return default_lut;

    auto & l = luts[lut_array_index];
    auto lut = l.lut.load(std::memory_order_consume);
    if (lut)
    {
        return *lut;
    }

    // Make DateLUTImpl, load data and store pointer atomically.
    auto new_lut = std::make_unique<const DateLUTImpl>(default_lut.getTimeZone(), l.min_time);
    const DateLUTImpl * old_lut = nullptr;
    if (l.lut.compare_exchange_strong(old_lut, new_lut.get(), std::memory_order_release, std::memory_order_consume))
    {
        // Stored by this tread
        return *new_lut.release();
    }
    else
    {
        // This LUT was already loaded and put to `luts` by another thread.
        new_lut.reset();
        return *old_lut;
    }
}
