#include "FunctionsStringSearch.h"
#include <Functions/FunctionFactory.h>
#include "HasTokenImpl.h"
#include <Common/Volnitsky.h>
#include <Storages/MergeTree/MergeTreeIndexFullText.h>

namespace DB
{
namespace
{

struct NameHasToken
{
    static constexpr auto name = "hasToken";
};

struct NameHasToken_v2
{
    static constexpr auto name = "hasToken_v2";
};

using FunctionHasToken = FunctionsStringSearch<HasTokenImpl<VolnitskyCaseSensitiveToken<SplitTokenExtractor>, NameHasToken, false>>;
using FunctionHasToken_v2 = FunctionsStringSearch<HasTokenImpl<VolnitskyCaseSensitiveToken<SplitTokenExtractor2>, NameHasToken_v2, false>>;

}

void registerFunctionHasToken(FunctionFactory & factory)
{
    factory.registerFunction<FunctionHasToken>();
    factory.registerFunction<FunctionHasToken_v2>();
}

}
