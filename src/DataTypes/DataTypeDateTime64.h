#pragma once

#include <Core/Types.h>
#include <DataTypes/DataTypeDateTime.h>
#include <DataTypes/DataTypeDecimalBase.h>
#include <common/TimeZone.h>

class DateLUTImpl;

namespace DB
{

/** DateTime64 is same as DateTime, but it stores values as Int64 and has configurable sub-second part.
 *
 * `scale` determines number of decimal places for sub-second part of the DateTime64.
  */
class DataTypeDateTime64 final : public DataTypeDecimalBase<DateTime64>, public TimezoneMixin
{
public:
    using Base = DataTypeDecimalBase<DateTime64>;
    static constexpr UInt8 default_scale = 3;

    static constexpr auto family_name = "DateTime64";
    static constexpr auto type_id = TypeIndex::DateTime64;

    explicit DataTypeDateTime64(UInt32 scale_, const std::string & time_zone_name = "");

    // reuse timezone from other DateTime/DateTime64
    DataTypeDateTime64(UInt32 scale_, const TimezoneMixin & time_zone_info);

    const char * getFamilyName() const override { return family_name; }
    std::string doGetName() const override;
    TypeIndex getTypeId() const override { return type_id; }

    void serializeText(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings &) const override;
    void deserializeText(IColumn & column, ReadBuffer & istr, const FormatSettings &) const override;
    void deserializeWholeText(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const override;
    void serializeTextEscaped(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings &) const override;
    void deserializeTextEscaped(IColumn & column, ReadBuffer & istr, const FormatSettings &) const override;
    void serializeTextQuoted(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings &) const override;
    void deserializeTextQuoted(IColumn & column, ReadBuffer & istr, const FormatSettings &) const override;
    void serializeTextJSON(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings &) const override;
    void deserializeTextJSON(IColumn & column, ReadBuffer & istr, const FormatSettings &) const override;
    void serializeTextCSV(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings &) const override;
    void deserializeTextCSV(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const override;
    void serializeProtobuf(const IColumn & column, size_t row_num, ProtobufWriter & protobuf, size_t & value_index) const override;
    void deserializeProtobuf(IColumn & column, ProtobufReader & protobuf, bool allow_add_row, bool & row_added) const override;

    bool equals(const IDataType & rhs) const override;

    bool canBePromoted() const override { return false; }
};

/** Tansform-type wrapper for DateTime64, simplifies DateTime64 support for given Transform.
 *
 * Depending on what overloads of Transform::execute() are available, when called with DateTime64 value,
 * invokes Transform::execute() with either:
 * * whole part of DateTime64 value, discarding fractional part (1)
 * * DateTime64 value and scale factor (2)
 * * DateTime64 broken down to components, result of execute is then re-assembled back into DateTime64 value (3)
 *
 * Suitable Transfotm-types are commonly used in Date/DateTime manipulation functions,
 * and should implement static (or const) function with following signatures:
 * 1:
 *     R execute(Int64 whole_value, ... , const TimeZone &)
 * 2:
 *     R execute(DateTime64 value, Int64 scale_multiplier, ... , const TimeZone &)
 * 3:
 *     DecimalUtils::DecimalComponents<DateTime64> execute(DecimalUtils::DecimalComponents<DateTime64> components, ... , const TimeZone &)
 *
 * Where R could be arbitrary type.
*/
template <typename Transform>
class TransformDateTime64
{
private:
    // Detect if Transform::execute is const or static method
    // with signature defined by template args (ignoring result type).
    template<typename = void, typename... Args>
    struct TransformHasExecuteOverload : std::false_type {};

    template<typename... Args>
    struct TransformHasExecuteOverload<std::void_t<decltype(std::declval<Transform>().execute(std::declval<Args>()...))>, Args...>
        : std::true_type {};

    template<typename... Args>
    static constexpr bool TransformHasExecuteOverload_v = TransformHasExecuteOverload<void, Args...>::value;

public:
    static constexpr auto name = Transform::name;

    // non-explicit constructor to allow creating from scale value (or with no scale at all), indispensable in some contexts.
    TransformDateTime64(UInt32 scale_ = 0)
        : scale_multiplier(DecimalUtils::scaleMultiplier<DateTime64::NativeType>(scale_))
    {}

    template <typename ... Args>
    inline auto execute(const DateTime64 & t, Args && ... args) const
    {
        if constexpr (TransformHasExecuteOverload_v<DateTime64, decltype(scale_multiplier), Args...>)
        {
            return wrapped_transform.execute(t, scale_multiplier, std::forward<Args>(args)...);
        }
        else if constexpr (TransformHasExecuteOverload_v<DecimalUtils::DecimalComponents<DateTime64>, Args...>)
        {
            auto components = DecimalUtils::splitWithScaleMultiplier(t, scale_multiplier);
            components = wrapped_transform.execute(components, std::forward<Args>(args)...);
            return DecimalUtils::decimalFromComponents<DateTime64>(components, scale_multiplier);
        }
        else
        {
            const auto components = DecimalUtils::splitWithScaleMultiplier(t, scale_multiplier);
            return wrapped_transform.execute(static_cast<Int64>(components.whole), std::forward<Args>(args)...);
        }
    }

    template <typename T, typename ... Args, typename = std::enable_if_t<std::negation_v<std::is_same_v<T, DateTime64>>>>
    inline auto execute(const T & t, Args && ... args) const
    {
        return wrapped_transform.execute(t, std::forward<Args>(args)...);
    }

private:
    DateTime64::NativeType scale_multiplier = 1;
    Transform wrapped_transform = {};
};

}

