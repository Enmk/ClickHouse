#include "MergeTreeDataPartDeletedMask.h"
#include <iterator>
#include <IO/ReadHelpers.h>
#include <IO/WriteHelpers.h>

namespace DB::ErrorCodes
{
    extern const int UNKNOWN_FORMAT_VERSION;
    extern const int HASH_MISMATCH_FOR_DELETED_MASK;
}

namespace DB
{
void MergeTreeDataPartDeletedMask::read(ReadBuffer & in)
{
    size_t format_version;

    readIntBinary(format_version, in);

    if (format_version != 1)
        throw Exception(ErrorCodes::UNKNOWN_FORMAT_VERSION,
            "Unknown format version {} for deleted mask", format_version);

    readIntBinary(deleted_rows_hash, in);

    readVectorBinary(deleted_rows, in);

    size_t real_hash = 0;

    for (size_t elem : deleted_rows)
        real_hash ^= hasher(elem);

    if (deleted_rows_hash != real_hash)
        throw Exception(ErrorCodes::HASH_MISMATCH_FOR_DELETED_MASK,
            "Hash mismatch for deleted mask, expected {}, found {}",
            deleted_rows_hash, real_hash);

    assertEOF(in);
}

void MergeTreeDataPartDeletedMask::write(WriteBuffer & out) const
{
    writeVarUInt(1, out); //format version
    writeVarUInt(deleted_rows_hash, out);
    writeBinary(std::span{deleted_rows}, out);
}

void MergeTreeDataPartDeletedMask::update(const MergeTreeDataPartDeletedMask& other)
{
    std::copy(other.deleted_rows.cbegin(), other.deleted_rows.cend(), std::back_inserter(deleted_rows));
    deleted_rows_hash ^= other.deleted_rows_hash;
}

void MergeTreeDataPartDeletedMask::updateWrite(const MergeTreeDataPartDeletedMask& other, WriteBufferFromFile & out)
{
    update(other);

    out.seek(sizeof(size_t), SEEK_CUR); //reposition on hash
    writeVarUInt(deleted_rows_hash, out);

    out.seek(0, SEEK_END); //reposition after previously added rows

    writeBinary(std::span{other.deleted_rows}, out);
}
}
