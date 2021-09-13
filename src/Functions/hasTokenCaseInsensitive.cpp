#include "FunctionsStringSearch.h"
#include <Functions/FunctionFactory.h>
#include "HasTokenImpl.h"
#include <Common/Volnitsky.h>
#include <Storages/MergeTree/MergeTreeIndexFullText.h>


namespace DB
{
namespace
{

struct NameHasTokenCaseInsensitive
{
    static constexpr auto name = "hasTokenCaseInsensitive";
};

using FunctionHasTokenCaseInsensitive
    = FunctionsStringSearch<HasTokenImpl<VolnitskyCaseInsensitiveToken<SplitTokenExtractor>, NameHasTokenCaseInsensitive, false>>;

}

void registerFunctionHasTokenCaseInsensitive(FunctionFactory & factory)
{
    factory.registerFunction<FunctionHasTokenCaseInsensitive>();
}

}
