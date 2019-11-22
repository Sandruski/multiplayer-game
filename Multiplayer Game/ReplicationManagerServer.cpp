#include "ReplicationManagerServer.h"
#include "Networks.h"

ReplicationManagerServer::ReplicationManagerServer()
    : m_replicationCommands()
{
}

ReplicationManagerServer::~ReplicationManagerServer()
{
}

void ReplicationManagerServer::create(uint32 networkID)
{
    m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Create, networkID));
}

void ReplicationManagerServer::update(uint32 networkID)
{
    m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Update, networkID));
}

void ReplicationManagerServer::destroy(uint32 networkID)
{
    m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Destroy, networkID));
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationManagerTransmissionData* replicationManagerTransmissionData)
{
    for (const auto& replicationCommand : m_replicationCommands) {

        uint32 networkID = replicationCommand.m_networkID;
        ReplicationAction action = replicationCommand.m_action;
        GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);

        switch (action) {
        case ReplicationAction::Create: {
        case ReplicationAction::Update: {
            if (gameObject == nullptr) {
                continue;
            }
            packet.Write(networkID);
            packet.Write(action);
            gameObject->write(packet);
            break;
        }

        case ReplicationAction::Destroy: {
            packet.Write(networkID);
            packet.Write(action);
            break;
        }

        default: {
            break;
        }
        }
        }

        replicationManagerTransmissionData->AddTransmission(replicationCommand);
    }

    m_replicationCommands.clear();
}

ReplicationCommand::ReplicationCommand(ReplicationAction action, uint32 networkID)
    : m_action(action)
    , m_networkID(networkID)
{
}

ReplicationManagerTransmissionData::ReplicationManagerTransmissionData(ReplicationManagerServer* replicationManager)
    : m_replicationManager(replicationManager)
{
}

ReplicationManagerTransmissionData::~ReplicationManagerTransmissionData()
{
}

void ReplicationManagerTransmissionData::onDeliverySuccess(DeliveryManager* deliveryManager)
{
    for (const auto& replicationCommand : m_replicationCommands) {
    }
}

void ReplicationManagerTransmissionData::onDeliveryFailure(DeliveryManager deliveryManager)
{
    for (const auto& replicationCommand : m_replicationCommands) {
    }
}

void ReplicationManagerTransmissionData::AddTransmission(const ReplicationCommand& replicationCommand)
{
    m_replicationCommands.push_back(replicationCommand);
}
