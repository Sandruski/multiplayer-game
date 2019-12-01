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
	m_replicationCommands[networkID] = ReplicationCommand(ReplicationAction::Create, networkID);
}

void ReplicationManagerServer::update(uint32 networkID)
{
	auto it = m_replicationCommands.find(networkID);
	if (it == m_replicationCommands.end())
	{
		create(networkID);
		return;
	}

	m_replicationCommands[networkID].m_action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkID)
{
	auto it = m_replicationCommands.find(networkID);
	if (it == m_replicationCommands.end())
	{
		return;
	}

	m_replicationCommands[networkID].m_action = ReplicationAction::Destroy;
}

void ReplicationManagerServer::remove(uint32 networkID)
{
	m_replicationCommands.erase(networkID);
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationManagerTransmissionData* replicationManagerTransmissionData)
{
    for (auto& replicationCommand : m_replicationCommands) {

        uint32 networkID = replicationCommand.second.m_networkID;
		packet.Write(networkID);
        ReplicationAction replicationAction = replicationCommand.second.m_action;
		packet.Write(replicationAction);

        switch (replicationAction) {
		case ReplicationAction::Create: 
		case ReplicationAction::Update: {
			writeCreateOrUpdate(packet, networkID);
			break;
        }

        default: {
            break;
        }
        }

        replicationManagerTransmissionData->AddTransmission(replicationCommand.second);

		replicationCommand.second.m_action = ReplicationAction::None;
    }
}

void ReplicationManagerServer::writeCreateOrUpdate(OutputMemoryStream& packet, uint32 networkID)
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	gameObject->write(packet);
}

ReplicationCommand::ReplicationCommand()
{
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
		switch (replicationCommand.m_action)
		{
		case ReplicationAction::Destroy:
		{
			HandleDestroyDeliveryFailure(replicationCommand.m_networkID);
			break;
		}

		default:
		{
			break;
		}
		}
	}
}

void ReplicationManagerTransmissionData::onDeliveryFailure(DeliveryManager* deliveryManager)
{
    for (const auto& replicationCommand : m_replicationCommands) {
		switch (replicationCommand.m_action)
		{
		case ReplicationAction::Create:
		{
			HandleCreateDeliveryFailure(replicationCommand.m_networkID);
			break;
		}

		case ReplicationAction::Update:
		{
			HandleUpdateDeliveryFailure(replicationCommand.m_networkID);
			break;
		}

		case ReplicationAction::Destroy:
		{
			HandleDestroyDeliveryFailure(replicationCommand.m_networkID);
			break;
		}

		default:
		{
			break;
		}
		}
    }
}

void ReplicationManagerTransmissionData::HandleDestroyDeliverySuccess(uint32 networkID) const
{
	m_replicationManager->remove(networkID);
}

void ReplicationManagerTransmissionData::HandleCreateDeliveryFailure(uint32 networkID) const
{
	if (App->modLinkingContext->getNetworkGameObject(networkID) != nullptr)
	{
		m_replicationManager->create(networkID);
	}
}

void ReplicationManagerTransmissionData::HandleUpdateDeliveryFailure(uint32 networkID) const
{
	if (App->modLinkingContext->getNetworkGameObject(networkID) != nullptr)
	{
		m_replicationManager->update(networkID);
	}
}

void ReplicationManagerTransmissionData::HandleDestroyDeliveryFailure(uint32 networkID) const
{
	if (App->modLinkingContext->getNetworkGameObject(networkID) == nullptr)
	{
		m_replicationManager->destroy(networkID);
	}
}

void ReplicationManagerTransmissionData::AddTransmission(const ReplicationCommand& replicationCommand)
{
    m_replicationCommands.push_back(replicationCommand);
}
