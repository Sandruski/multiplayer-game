#pragma once

enum class ReplicationAction {
    None,
    Create,
    Update,
    Destroy
};

struct ReplicationCommand {
public:
	ReplicationCommand();
    ReplicationCommand(ReplicationAction action, uint32 networkID);

public:
    ReplicationAction m_action;
    uint32 m_networkID;
};

class ReplicationManagerTransmissionData;

class ReplicationManagerServer {
public:
    ReplicationManagerServer();
    ~ReplicationManagerServer();

    void create(uint32 networkID);
    void update(uint32 networkID);
    void destroy(uint32 networkID);

	void remove(uint32 networkID);

    void write(OutputMemoryStream& packet, ReplicationManagerTransmissionData* replicationManagerTransmissionData);

	void writeCreateOrUpdate(OutputMemoryStream& packet, uint32 networkID);

private:
	std::unordered_map<uint32, ReplicationCommand> m_replicationCommands;
};

class ReplicationManagerTransmissionData : public DeliveryDelegate {
public:
    ReplicationManagerTransmissionData(ReplicationManagerServer* replicationManager);
    ~ReplicationManagerTransmissionData();

    void onDeliverySuccess(DeliveryManager* deliveryManager) override;
    void onDeliveryFailure(DeliveryManager* deliveryManager) override;

	void HandleDestroyDeliverySuccess(uint32 networkID) const;

	void HandleCreateDeliveryFailure(uint32 networkID) const;
	void HandleUpdateDeliveryFailure(uint32 networkID) const;
	void HandleDestroyDeliveryFailure(uint32 networkID) const;

    void AddTransmission(const ReplicationCommand& replicationCommand);

private:
    ReplicationManagerServer* m_replicationManager;
    std::vector<ReplicationCommand> m_replicationCommands;
};
