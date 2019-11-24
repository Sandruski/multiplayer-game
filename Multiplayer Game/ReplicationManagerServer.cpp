#include "ReplicationManagerServer.h"
#include "Networks.h"

ReplicationManagerServer::ReplicationManagerServer()
    : m_replicationCommands()
{
}

ReplicationManagerServer::~ReplicationManagerServer()
{
}

void ReplicationManagerServer::removeFromReplication(uint32 networkID)
{
	for (auto it = m_replicationCommands.begin(); it != m_replicationCommands.end(); ++it)
	{
		if ((*it).m_networkID == networkID)
		{
			m_replicationCommands.erase(it);
			break;
		}
	}
}

void ReplicationManagerServer::handleCreateAckd(uint32 networkID)
{
	for (auto& replicationCommand : m_replicationCommands)
	{
		if (replicationCommand.m_networkID == networkID)
		{
			if (replicationCommand.m_action == ReplicationAction::Create)
			{
				replicationCommand.m_action = ReplicationAction::Update;
			}
			break;
		}
	}
}

void ReplicationManagerServer::create(uint32 networkID)
{
	for (auto& replicationCommand : m_replicationCommands)
	{
		if (replicationCommand.m_networkID == networkID)
		{
			replicationCommand.m_action = ReplicationAction::Create;
			return;
		}
	}

    m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Create, networkID));
}

void ReplicationManagerServer::update(uint32 networkID)
{
	// handleCreateAckd does the job
}

void ReplicationManagerServer::destroy(uint32 networkID)
{
	for (auto& replicationCommand : m_replicationCommands)
	{
		if (replicationCommand.m_networkID == networkID)
		{
			replicationCommand.m_action = ReplicationAction::Destroy;
			return;
		}
	}
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationManagerTransmissionData* replicationManagerTransmissionData)
{
    for (const auto& replicationCommand : m_replicationCommands) {

        uint32 networkID = replicationCommand.m_networkID;
		packet.Write(networkID);
        ReplicationAction replicationAction = replicationCommand.m_action;
		packet.Write(replicationAction);

        switch (replicationAction) {
        case ReplicationAction::Create: {
        case ReplicationAction::Update: {
			writeCreateOrUpdate(packet, networkID);
            break;
        }

        default: {
            break;
        }
        }
        }

        replicationManagerTransmissionData->AddTransmission(replicationCommand);
    }
}

void ReplicationManagerServer::writeCreateOrUpdate(OutputMemoryStream& packet, uint32 networkID)
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	gameObject->write(packet);
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
		case ReplicationAction::Create:
		{
			HandleCreateDeliverySuccess(replicationCommand.m_networkID);
			break;
		}

		case ReplicationAction::Destroy:
		{
			HandleDestroyDeliverySuccess(replicationCommand.m_networkID);
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

void ReplicationManagerTransmissionData::HandleCreateDeliverySuccess(uint32 networkID) const
{
	m_replicationManager->handleCreateAckd(networkID);
}

void ReplicationManagerTransmissionData::HandleDestroyDeliverySuccess(uint32 networkID) const
{
	m_replicationManager->removeFromReplication(networkID);
}

void ReplicationManagerTransmissionData::HandleCreateDeliveryFailure(uint32 networkID) const
{
	if (App->modLinkingContext->getNetworkGameObject(networkID) != nullptr)
	{
		m_replicationManager->create(networkID);
	}
}

void ReplicationManagerTransmissionData::HandleDestroyDeliveryFailure(uint32 networkID) const
{
	m_replicationManager->destroy(networkID);
}

void ReplicationManagerTransmissionData::AddTransmission(const ReplicationCommand& replicationCommand)
{
    m_replicationCommands.push_back(replicationCommand);
}
