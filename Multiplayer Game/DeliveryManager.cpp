#include "DeliveryManager.h"
#include "Networks.h"

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
    uint32 sequenceNumber = m_nextOutgoingSequenceNumber++;
    packet.Write(sequenceNumber);

    Delivery* delivery = new Delivery(sequenceNumber);
    m_pendingDeliveries.push_back(delivery);

    return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
    return false;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
    return false;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
}

void DeliveryManager::processAckSequenceNumbers(const InputMemoryStream& packet)
{
}

void DeliveryManager::processTimedOutPackets()
{
}

void DeliveryManager::clear()
{
}

Delivery::Delivery(uint32 sequenceNumber)
    : sequenceNumber(sequenceNumber)
{
}

Delivery::~Delivery()
{
}
