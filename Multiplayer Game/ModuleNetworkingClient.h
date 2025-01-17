#pragma once

#include "ModuleNetworking.h"
#include "ReplicationManagerClient.h"

class ModuleNetworkingClient : public ModuleNetworking {
public:
    //////////////////////////////////////////////////////////////////////
    // ModuleNetworkingClient public methods
    //////////////////////////////////////////////////////////////////////

    void setServerAddress(const char* serverAddress, uint16 serverPort);

    void setPlayerInfo(const char* playerName, uint8 spaceshipType);

	uint32 GetSpaceshipNetworkID() { return networkId; }

	float ComputeAverageReplicationTime() const;

	bool IsEntityInterpolationEnabled() const { return bEntityInterpolation; }

	void EnableEntityInterpolation() { bEnableEntityInterpolationAtNextRepl = true; }

private:
    //////////////////////////////////////////////////////////////////////
    // ModuleNetworking virtual methods
    //////////////////////////////////////////////////////////////////////

    bool isClient() const override { return true; }

    void onStart() override;

    void onGui() override;

    void onPacketReceived(const InputMemoryStream& packet, const sockaddr_in& fromAddress) override;

    void onUpdate() override;

    void onConnectionReset(const sockaddr_in& fromAddress) override;

    void onDisconnect() override;

    //////////////////////////////////////////////////////////////////////
    // Client state
    //////////////////////////////////////////////////////////////////////

    enum class ClientState {
        Stopped,
        Start,
        WaitingWelcome,
        Playing
    };

    ClientState state = ClientState::Stopped;

    std::string serverAddressStr;
    uint16 serverPort = 0;

    sockaddr_in serverAddress = {};
    std::string playerName = "player";
    uint8 spaceshipType = 0;

    uint32 playerId = 0;
    uint32 networkId = 0;


	static const int MAX_REPLICATION_TIME_BUFFER = 8;
	float m_replicationTimeBuffer[MAX_REPLICATION_TIME_BUFFER];
	uint32 m_replicationTimeFront = 0;

    static const int MAX_INPUT_DATA_SIMULTANEOUS_PACKETS = 64;

    // Queue of input data
    InputPacketData inputData[MAX_INPUT_DATA_SIMULTANEOUS_PACKETS];
    uint32 inputDataFront = 0;
    uint32 inputDataBack = 0;

    float inputDeliveryIntervalSeconds = 0.05f;
    float secondsSinceLastInputDelivery = 0.0f;

	bool bClientPrediction = true;

	bool bEntityInterpolation = true;
	bool bEnableEntityInterpolationAtNextRepl = true;
    // Timeout / ping

    double lastPacketReceivedTime = 0.0f; // NOTE(jesus): Use this to implement client timeout
    float secondsSinceLastPing = 0.0f; // NOTE(jesus): Use this to implement ping to server

    ReplicationManagerClient m_replicationManager;
    DeliveryManager m_deliveryManager;
};
