#pragma once

#include <common/types.h>
#include <Interpreters/Session.h>
#include <Core/MySQL/PacketEndpoint.h>

namespace DB
{

class MySQLSession : public DB::Session
{
public:
    using DB::Session::Session;
    MySQLWireContext mysql_context;
};

}
