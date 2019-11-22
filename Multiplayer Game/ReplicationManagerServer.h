#pragma once

enum class ReplicationAction {
    None,
    Create,
    Update,
    Destroy
};

struct ReplicationCommand {
public:
    ReplicationCommand(ReplicationAction action, uint32 networkID);

public:
    ReplicationAction m_action;
    uint32 m_networkID;
};

class ReplicationManagerServer {
public:
    ReplicationManagerServer();
    ~ReplicationManagerServer();

    void create(uint32 networkID);
    void update(uint32 networkID);
    void destroy(uint32 networkID);

    void write(OutputMemoryStream& packet);

private:
    std::vector<ReplicationCommand> m_replicationCommands;
};
