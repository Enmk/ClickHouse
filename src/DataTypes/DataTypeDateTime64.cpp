#include <DataTypes/DataTypeDateTime64.h>

#include <Columns/ColumnVector.h>
#include <Common/assert_cast.h>
#include <Common/typeid_cast.h>
#include <common/DateLUT.h>
#include <DataTypes/DataTypeFactory.h>
#include <Formats/FormatSettings.h>
#include <Formats/ProtobufReader.h>
#include <Formats/ProtobufWriter.h>
#include <IO/Operators.h>
#include <IO/ReadHelpers.h>
#include <IO/WriteBufferFromString.h>
#include <IO/WriteHelpers.h>
#include <IO/parseDateTimeBestEffort.h>
#include <Parsers/ASTLiteral.h>

#include <optional>
#include <string>


namespace DB
{

namespace ErrorCodes
{
    extern const int ARGUMENT_OUT_OF_BOUND;
}

static constexpr UInt32 max_scale = 9;

DataTypeDateTime64::DataTypeDateTime64(UInt32 scale_, const std::string & time_zone_name)
    : DataTypeDecimalBase<DateTime64>(DecimalUtils::maxPrecision<DateTime64>(), scale_),
      TimezoneMixin(time_zone_name)
{
    if (scale > max_scale)
        throw Exception("Scale " + std::to_string(scale) + " is too large for DateTime64. Maximum is up to nanoseconds (9).",
            ErrorCodes::ARGUMENT_OUT_OF_BOUND);
}

DataTypeDateTime64::DataTypeDateTime64(UInt32 scale_, const TimezoneMixin & time_zone_info)
    : DataTypeDecimalBase<DateTime64>(DecimalUtils::maxPrecision<DateTime64>(), scale_),
      TimezoneMixin(time_zone_info)
{
    if (scale > max_scale)
        throw Exception("Scale " + std::to_string(scale) + " is too large for DateTime64. Maximum is up to nanoseconds (9).",
            ErrorCodes::ARGUMENT_OUT_OF_BOUND);
}

std::string DataTypeDateTime64::doGetName() const
{
    if (!has_explicit_time_zone)
        return std::string(getFamilyName()) + "(" + std::to_string(this->scale) + ")";

    WriteBufferFromOwnString out;
    out << "DateTime64(" << this->scale << ", " << quote << time_zone.getTimeZone() << ")";
    return out.str();
}

void DataTypeDateTime64::serializeText(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings & settings) const
{
    auto value = assert_cast<const ColumnType &>(column).getData()[row_num];
    switch (settings.date_time_output_format)
    {
        case FormatSettings::DateTimeOutputFormat::Simple:
            writeDateTimeText(value, scale, ostr, time_zone);
            return;
        case FormatSettings::DateTimeOutputFormat::UnixTimestamp:
            writeDateTimeUnixTimestamp(value, scale, ostr);
            return;
        case FormatSettings::DateTimeOutputFormat::ISO:
            writeDateTimeTextISO(value, scale, ostr, utc_time_zone);
            return;
    }
}

void DataTypeDateTime64::deserializeText(IColumn & column, ReadBuffer & istr, const FormatSettings &) const
{
    DateTime64 result = 0;
    readDateTime64Text(result, this->getScale(), istr, time_zone);
    assert_cast<ColumnType &>(column).getData().push_back(result);
}

void DataTypeDateTime64::deserializeWholeText(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const
{
    deserializeTextEscaped(column, istr, settings);
}

void DataTypeDateTime64::serializeTextEscaped(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings & settings) const
{
    serializeText(column, row_num, ostr, settings);
}

static inline void readText(DateTime64 & x, UInt32 scale, ReadBuffer & istr, const FormatSettings & settings, const TimeZone & time_zone, const TimeZone & utc_time_zone)
{
    switch (settings.date_time_input_format)
    {
        case FormatSettings::DateTimeInputFormat::Basic:
            readDateTime64Text(x, scale, istr, time_zone);
            return;
        case FormatSettings::DateTimeInputFormat::BestEffort:
            parseDateTime64BestEffort(x, scale, istr, time_zone, utc_time_zone);
            return;
    }
}

void DataTypeDateTime64::deserializeTextEscaped(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const
{
    DateTime64 x = 0;
    readText(x, scale, istr, settings, time_zone, utc_time_zone);
    assert_cast<ColumnType &>(column).getData().push_back(x);
}

void DataTypeDateTime64::serializeTextQuoted(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings & settings) const
{
    writeChar('\'', ostr);
    serializeText(column, row_num, ostr, settings);
    writeChar('\'', ostr);
}

void DataTypeDateTime64::deserializeTextQuoted(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const
{
    DateTime64 x = 0;
    if (checkChar('\'', istr)) /// Cases: '2017-08-31 18:36:48' or '1504193808'
    {
        readText(x, scale, istr, settings, time_zone, utc_time_zone);
        assertChar('\'', istr);
    }
    else /// Just 1504193808 or 01504193808
    {
        readIntText(x, istr);
    }
    assert_cast<ColumnType &>(column).getData().push_back(x);    /// It's important to do this at the end - for exception safety.
}

void DataTypeDateTime64::serializeTextJSON(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings & settings) const
{
    writeChar('"', ostr);
    serializeText(column, row_num, ostr, settings);
    writeChar('"', ostr);
}

void DataTypeDateTime64::deserializeTextJSON(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const
{
    DateTime64 x = 0;
    if (checkChar('"', istr))
    {
        readText(x, scale, istr, settings, time_zone, utc_time_zone);
        assertChar('"', istr);
    }
    else
    {
        readIntText(x, istr);
    }
    assert_cast<ColumnType &>(column).getData().push_back(x);
}

void DataTypeDateTime64::serializeTextCSV(const IColumn & column, size_t row_num, WriteBuffer & ostr, const FormatSettings & settings) const
{
    writeChar('"', ostr);
    serializeText(column, row_num, ostr, settings);
    writeChar('"', ostr);
}

void DataTypeDateTime64::deserializeTextCSV(IColumn & column, ReadBuffer & istr, const FormatSettings & settings) const
{
    DateTime64 x = 0;

    if (istr.eof())
        throwReadAfterEOF();

    char maybe_quote = *istr.position();

    if (maybe_quote == '\'' || maybe_quote == '\"')
        ++istr.position();

    readText(x, scale, istr, settings, time_zone, utc_time_zone);

    if (maybe_quote == '\'' || maybe_quote == '\"')
        assertChar(maybe_quote, istr);

    assert_cast<ColumnType &>(column).getData().push_back(x);
}

void DataTypeDateTime64::serializeProtobuf(const IColumn & column, size_t row_num, ProtobufWriter & protobuf, size_t & value_index) const
{
    if (value_index)
        return;
    value_index = static_cast<bool>(protobuf.writeDateTime64(assert_cast<const ColumnType &>(column).getData()[row_num], scale));
}

void DataTypeDateTime64::deserializeProtobuf(IColumn & column, ProtobufReader & protobuf, bool allow_add_row, bool & row_added) const
{
    row_added = false;
    DateTime64 t = 0;
    if (!protobuf.readDateTime64(t, scale))
        return;

    auto & container = assert_cast<ColumnType &>(column).getData();
    if (allow_add_row)
    {
        container.emplace_back(t);
        row_added = true;
    }
    else
        container.back() = t;
}

bool DataTypeDateTime64::equals(const IDataType & rhs) const
{
    if (const auto * ptype = typeid_cast<const DataTypeDateTime64 *>(&rhs))
        return this->scale == ptype->getScale();
    return false;
}

}
