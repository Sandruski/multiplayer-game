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
            GameObject* gameObject = App->modGameObject->Instantiate();
            App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkID);
            gameObject->read(packet);
            break;
        }

        case ReplicationAction::Update: {
            GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
            gameObject->read(packet);
            break;
        }

        case ReplicationAction::Destroy: {
            GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkID);
            App->modLinkingContext->unregisterNetworkGameObject(gameObject);
            App->modGameObject->Destroy(gameObject);
            break;
        }

        default: {
            break;
        }
        }
    }
}
