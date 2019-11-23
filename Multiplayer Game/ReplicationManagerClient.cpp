#include "ReplicationManagerClient.h"
#include "Networks.h"

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
    while (packet.RemainingByteCount() > 0){

        uint32 networkID;
        packet.Read(networkID);
        ReplicationAction action;
        packet.Read(action);

        switch (action) {
        case ReplicationAction::Create: {
			create(packet, networkID);
            break;
        }

        case ReplicationAction::Update: {
			update(packet, networkID);
            break;
        }

        case ReplicationAction::Destroy: {
			destroy(networkID);
            break;
        }

        default: {
            break;
        }
        }
    }
}

void ReplicationManagerClient::create(const InputMemoryStream& packet, uint32 networkID) const
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	if (gameObject == nullptr)
	{
		GameObject* newGameObject = App->modGameObject->Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(newGameObject, networkID);
		gameObject = newGameObject;
	}

	gameObject->read(packet);
}

void ReplicationManagerClient::update(const InputMemoryStream& packet, uint32 networkID) const
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	if (gameObject != nullptr)
	{
		gameObject->read(packet);
	}
}

void ReplicationManagerClient::destroy(uint32 networkID) const
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	if (gameObject != nullptr)
	{
		App->modLinkingContext->unregisterNetworkGameObject(gameObject);
		App->modGameObject->Destroy(gameObject);
	}
}
