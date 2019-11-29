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
		gameObject = App->modLinkingContext->getNetworkGameObjectUnsafe(networkID);
		if (gameObject != nullptr)
		{
			App->modLinkingContext->unregisterNetworkGameObject(gameObject);
			Destroy(gameObject);
		}

		gameObject = App->modGameObject->Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkID);

		if (gameObject->networkId == App->modNetClient->GetSpaceshipNetworkID())
			gameObject->isClientSS = true;
	}

	gameObject->read(packet);
}

void ReplicationManagerClient::update(const InputMemoryStream& packet, uint32 networkID) const
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	if (gameObject == nullptr)
	{
		GameObject auxGameObject;
		auxGameObject.read(packet);
		auxGameObject.releaseComponents();
	}
	else
	{
		gameObject->read(packet);
		gameObject->interpolation.lerpMaxTime = App->modNetClient->ComputeAverageReplicationTime();
		gameObject->interpolation.secondsElapsed = 0.0f;
		gameObject->interpolation.initialPosition = gameObject->interpolation.prevPosition;
		gameObject->interpolation.finalPosition = gameObject->position;
		gameObject->position = gameObject->interpolation.initialPosition;
	}
}

void ReplicationManagerClient::destroy(uint32 networkID) const
{
	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
	if (gameObject != nullptr)
	{
		App->modLinkingContext->unregisterNetworkGameObject(gameObject);
		Destroy(gameObject);
	}
}
