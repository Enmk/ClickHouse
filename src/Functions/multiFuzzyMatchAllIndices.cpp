#include "FunctionsMultiStringFuzzySearch.h"
#include "FunctionFactory.h"
#include "MultiMatchAllIndicesImpl.h"


namespace DB
{
namespace
{

struct NameMultiFuzzyMatchAllIndices
{
    static constexpr auto name = "multiFuzzyMatchAllIndices";
};

using FunctionMultiFuzzyMatchAllIndices = FunctionsMultiStringFuzzySearch<
    MultiMatchAllIndicesImpl<UInt64, NameMultiFuzzyMatchAllIndices, true>,
    std::numeric_limits<UInt32>::max()>;

}

void registerFunctionMultiFuzzyMatchAllIndices(FunctionFactory & factory)
{
    factory.registerFunction<FunctionMultiFuzzyMatchAllIndices>();
}

}
