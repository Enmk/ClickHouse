#!/usr/bin/env bash

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
# shellcheck source=../shell_config.sh
. "$CURDIR"/../shell_config.sh

# TOKEN BF
$CLICKHOUSE_CLIENT -n --query="
CREATE TABLE bloom_filter_idx3
(
    k UInt64,
    s String,
    INDEX bf (s, lower(s)) TYPE tokenbf_v2(512, 3, 0) GRANULARITY 1
) ENGINE = MergeTree()
ORDER BY k
SETTINGS index_granularity = 2;"

$CLICKHOUSE_CLIENT --query="INSERT INTO bloom_filter_idx3 VALUES
(0, 'ClickHouse is a column-oriented database management system (DBMS) for online analytical processing of queries (OLAP).'),
(1, 'column-oriented'),
(2, 'column-oriented'),
(3, 'by_underscore'),
(4, 'snake_case'),
(6, 'some string'),
(8, 'column with ints'),
(9, '2_2%2_2\\\\'),
(13, 'abc'),
(14, 'some_text with words separated by_underscore')"

echo EQUAL
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE lower(s) = 'snake_case' OR s = 'by_underscore' ORDER BY k"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE lower(s) = 'snake_case' OR s = 'by_underscore' ORDER BY k FORMAT JSON" | grep "rows_read"

echo LIKE # does LIKE work on partial match of a token with underscore ???
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE lower(s) LIKE '%(by_underscore)%' ORDER BY k"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE lower(s) LIKE '%(by_underscore)%' ORDER BY k FORMAT JSON" | grep "rows_read"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE s LIKE 'by_%' AND s LIKE '%_underscore' ORDER BY k"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE s LIKE 'by_%' AND s LIKE '%_underscore' ORDER BY k FORMAT JSON" | grep "rows_read"

echo IN
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE s IN ('by_underscore', 'abc') ORDER BY k"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE s IN ('by_underscore', 'abc') ORDER BY k FORMAT JSON" | grep "rows_read"

echo hasToken_v2
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE hasToken_v2(s, 'by_underscore') ORDER BY k"
$CLICKHOUSE_CLIENT --query="SELECT * FROM bloom_filter_idx3 WHERE hasToken_v2(s, 'by_underscore') ORDER BY k FORMAT JSON" | grep "rows_read"

$CLICKHOUSE_CLIENT --query="DROP TABLE bloom_filter_idx3"

$CLICKHOUSE_CLIENT --query="DROP TABLE IF EXISTS bloom_filter_idx_na;"
$CLICKHOUSE_CLIENT -n --query="
CREATE TABLE bloom_filter_idx_na
(
    na Array(Array(String)),
    INDEX bf na TYPE bloom_filter(0.1) GRANULARITY 1
) ENGINE = MergeTree()
ORDER BY na" 2>&1 | grep -c 'DB::Exception: Unexpected type Array(Array(String)) of bloom filter index'
