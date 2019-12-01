#include "ModuleNetworkingClient.h"
#include "Networks.h"
#include "ModuleNetworkingClient.h"

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::setServerAddress(const char* pServerAddress, uint16 pServerPort)
{
    serverAddressStr = pServerAddress;
    serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char* pPlayerName, uint8 pSpaceshipType)
{
    playerName = pPlayerName;
    spaceshipType = pSpaceshipType;
}

float ModuleNetworkingClient::ComputeAverageReplicationTime() const
{
	float avr = 0.0f;
	for (int i = 0; i < MAX_REPLICATION_TIME_BUFFER; ++i)
	{
		avr += m_replicationTimeBuffer[i];
	}
	return avr / MAX_REPLICATION_TIME_BUFFER;
}


//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
    if (!createSocket())
        return;

    if (!bindSocketToPort(0)) {
        disconnect();
        return;
    }

    // Create remote address
    serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
    if (res == SOCKET_ERROR) {
        reportError("ModuleNetworkingClient::startClient() - inet_pton");
        disconnect();
        return;
    }

    state = ClientState::Start;

    inputDataFront = 0;
    inputDataBack = 0;
	memset(m_replicationTimeBuffer, 0, sizeof(float) * MAX_REPLICATION_TIME_BUFFER);
    secondsSinceLastInputDelivery = 0.0f;
    secondsSinceLastPing = 0.0f;
    lastPacketReceivedTime = Time.time;
}

void ModuleNetworkingClient::onGui()
{
    if (state == ClientState::Stopped)
        return;

    if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (state == ClientState::WaitingWelcome) {
            ImGui::Text("Waiting for server response...");
        } else if (state == ClientState::Playing) {
            ImGui::Text("Connected to server");

            ImGui::Separator();

            ImGui::Text("Player info:");
            ImGui::Text(" - Id: %u", playerId);
            ImGui::Text(" - Name: %s", playerName.c_str());

            ImGui::Separator();

            ImGui::Text("Spaceship info:");
            ImGui::Text(" - Type: %u", spaceshipType);
            ImGui::Text(" - Network id: %u", networkId);

            vec2 playerPosition = {};
            GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
            if (playerGameObject != nullptr) {
                playerPosition = playerGameObject->position;
            }
            ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

            ImGui::Separator();

            ImGui::Text("Connection checking info:");
            ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
            ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

            ImGui::Separator();

            ImGui::Text("Input:");
            ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);

			ImGui::Checkbox("Client prediction", &bClientPrediction);

			ImGui::Checkbox("Entity Interpolation", &bEntityInterpolation);
        }
    }

	if (state == ClientState::Playing) {
		ImGui::SetNextWindowPos(ImVec2(850.0f, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(100.0f, 50.0f));
		if (ImGui::Begin("KILLS"))
		{
			GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr)
			{
				ImGui::Text("%u", playerGameObject->kills);
			}

			ImGui::End();
		}
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream& packet, const sockaddr_in& fromAddress)
{
    lastPacketReceivedTime = Time.time;

    ServerMessage message;
    packet >> message;

    if (state == ClientState::WaitingWelcome) {
        if (message == ServerMessage::Welcome) {
            packet >> playerId;
            packet >> networkId;

            LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
            state = ClientState::Playing;
        } else if (message == ServerMessage::Unwelcome) {
            WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
            disconnect();
        }
    } else if (state == ClientState::Playing) {
        // TODO(jesus): Handle incoming messages from server
        if (message == ServerMessage::Ping)
            lastPacketReceivedTime = Time.time;
        else if (message == ServerMessage::Replication) {
            if (m_deliveryManager.processSequenceNumber(packet)) {
				uint32 nextExpectedInputSequenceNumber;
				packet.Read(nextExpectedInputSequenceNumber);
				inputDataFront = nextExpectedInputSequenceNumber;

                m_replicationManager.read(packet);	

				if (bClientPrediction)
				{
					GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
					for (uint32 i = inputDataFront; i < inputDataBack; ++i) {
						InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
						InputController controller;
						controller.horizontalAxis = inputPacketData.horizontalAxis;
						controller.horizontalAxis = inputPacketData.verticalAxis;
						unpackInputControllerButtons(inputPacketData.buttonBits, controller);
						if (playerGameObject != nullptr) {
							playerGameObject->behaviour->onInput(controller, true);
						}
					}
				}

				if (bEntityInterpolation)
				{
					m_replicationTimeFront += 1;
					float& currReplTime = m_replicationTimeBuffer[m_replicationTimeFront % ArrayCount(m_replicationTimeBuffer)];
					currReplTime = 0.0f;
				}

				if (m_deliveryManager.hasSequenceNumbersPendingAck()) {
					OutputMemoryStream stream;
					stream << ClientMessage::Ack;
					m_deliveryManager.writeSequenceNumbersPendingAck(stream);
					sendPacket(stream, fromAddress);
				}
            }
        }
    }
}

void ModuleNetworkingClient::onUpdate()
{
    if (state == ClientState::Stopped)
        return;

    if (state == ClientState::Start) {
        // Send the hello packet with player data

        OutputMemoryStream stream;
        stream << ClientMessage::Hello;
        stream << playerName;
        stream << spaceshipType;

        sendPacket(stream, serverAddress);

        state = ClientState::WaitingWelcome;
    } else if (state == ClientState::WaitingWelcome) {
    } else if (state == ClientState::Playing) {
        secondsSinceLastInputDelivery += Time.deltaTime;

        if (inputDataBack - inputDataFront < ArrayCount(inputData)) {
            uint32 currentInputData = inputDataBack++;
            InputPacketData& inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
            inputPacketData.sequenceNumber = currentInputData;
            inputPacketData.horizontalAxis = Input.horizontalAxis;
            inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);

			if (bClientPrediction)
			{
				GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
				if (playerGameObject != nullptr) {
					playerGameObject->behaviour->onInput(Input, true);
				}
			}

            // Create packet (if there's input and the input delivery interval exceeded)
            if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds) {
                secondsSinceLastInputDelivery = 0.0f;

                OutputMemoryStream packet;
                packet << ClientMessage::Input;

                for (uint32 i = inputDataFront; i < inputDataBack; ++i) {
                    InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
                    packet << inputPacketData.sequenceNumber;
                    packet << inputPacketData.horizontalAxis;
                    packet << inputPacketData.verticalAxis;
                    packet << inputPacketData.buttonBits;
                }

                // Clear the queue
                //inputDataFront = inputDataBack;

                sendPacket(packet, serverAddress);
            }
        }

        secondsSinceLastPing += Time.deltaTime;

        if (secondsSinceLastPing >= PING_INTERVAL_SECONDS) {
			secondsSinceLastPing = 0.0f;

            OutputMemoryStream packet;
            packet << ClientMessage::Ping;
            sendPacket(packet, serverAddress);
        }

        if (Time.time - lastPacketReceivedTime >= DISCONNECT_TIMEOUT_SECONDS)
            disconnect();

		if (bEntityInterpolation)
		{
			float& currReplTime = m_replicationTimeBuffer[m_replicationTimeFront % ArrayCount(m_replicationTimeBuffer)];
			currReplTime += Time.deltaTime;
		}
    }

	// Make the camera focus the player game object
	GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
	if (playerGameObject != nullptr) {
		App->modRender->cameraPosition = playerGameObject->position;
	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in& fromAddress)
{
    disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
    state = ClientState::Stopped;

    // Get all network objects and clear the linking context
    uint16 networkGameObjectsCount;
    GameObject* networkGameObjects[MAX_NETWORK_OBJECTS] = {};
    App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
    App->modLinkingContext->clear();

	m_deliveryManager.clear();

    // Destroy all network objects
    for (uint32 i = 0; i < networkGameObjectsCount; ++i) {
        Destroy(networkGameObjects[i]);
    }

    App->modRender->cameraPosition = {};
}