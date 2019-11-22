#pragma once

enum class ClientMessage {
    Hello,
    Input,
    Ping,
    Ack
};

enum class ServerMessage {
    Welcome,
    Unwelcome,
    Replication,
    Ping
};
