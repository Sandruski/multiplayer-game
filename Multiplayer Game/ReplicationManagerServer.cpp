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
	for (auto& replicationCommand : m_replicationCommands)
	{
		if (replicationCommand.m_networkID == networkID)
		{
			if (replicationCommand.m_action != ReplicationAction::Create)
			{
				replicationCommand.m_action = ReplicationAction::Update;
			}
			return;
		}
	}

	m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Update, networkID));
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

	m_replicationCommands.push_back(ReplicationCommand(ReplicationAction::Destroy, networkID));
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationManagerTransmissionData* replicationManagerTransmissionData)
{
    for (const auto& replicationCommand : m_replicationCommands) {

        uint32 networkID = replicationCommand.m_networkID;
		packet.Write(networkID);
        ReplicationAction replicationAction = replicationCommand.m_action;
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

        replicationManagerTransmissionData->AddTransmission(replicationCommand);
    }

	m_replicationCommands.clear();
}

void ReplicationManagerServer::writeCreateOrUpdate(OutputMemoryStream& packet, uint32 networkID)
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	gameObject->write(packet);
}

bool ReplicationManagerServer::hasPendingReplicationCommandsToWrite() const
{
	return !m_replicationCommands.empty();
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
}

void ReplicationManagerTransmissionData::onDeliveryFailure(DeliveryManager* deliveryManager)
{
	/*
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
    }*/
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
	if (App->modLinkingContext->getNetworkGameObject(networkID) == nullptr)
	{
		m_replicationManager->destroy(networkID);
	}
}

void ReplicationManagerTransmissionData::AddTransmission(const ReplicationCommand& replicationCommand)
{
    m_replicationCommands.push_back(replicationCommand);
}
