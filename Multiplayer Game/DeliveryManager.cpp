#include "DeliveryManager.h"
#include "Networks.h"

#define ACK_TIMEOUT 0.3f

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
    uint32 sequenceNumber = m_nextOutgoingSequenceNumber++;
    packet.Write(sequenceNumber);

    Delivery* delivery = new Delivery(sequenceNumber);
    m_pendingDeliveries.push_back(delivery);

    return delivery;
}

bool DeliveryManager::processSequenceNumber(uint32 sequenceNumber)
{
    if (sequenceNumber >= m_nextExpectedSequenceNumber) {
        m_pendingAcks.push_back(sequenceNumber);
        m_nextExpectedSequenceNumber = sequenceNumber + 1;
        return true;
    }

    return false;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
    return !m_pendingAcks.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
    packet.Write(m_pendingAcks.size());
    for (const auto& sequenceNumber : m_pendingAcks) {
        packet.Write(sequenceNumber);
    }
}

void DeliveryManager::processAckSequenceNumbers(const InputMemoryStream& packet)
{
    std::size_t size;
    packet.Read(size);
    for (uint32 i = 0; i < size; ++i) {
        uint32 sequenceNumber;
        packet.Read(sequenceNumber);

        /*
        const Delivery& delivery = m_deliveries.front();
        SequenceNumber inFlightPacketSequenceNumber = delivery.GetSequenceNumber();
        if (inFlightPacketSequenceNumber >= sequenceNumber) {
            HandleDeliverySuccess(delivery);
            m_deliveries.pop_front();
            sequenceNumber = inFlightPacketSequenceNumber + 1;
        } else if (inFlightPacketSequenceNumber < sequenceNumber) {
            HandleDeliveryFailure(delivery);
            m_deliveries.pop_front();
        }
		*/
    }
}

void DeliveryManager::processTimedOutPackets()
{
    float timeout = Time.time - ACK_TIMEOUT;
    while (!m_pendingDeliveries.empty()) {
        Delivery* delivery = m_pendingDeliveries.front();
        if (delivery->dispatchTime < timeout) {
            delivery->delegate->onDeliveryFailure(this);
            m_pendingDeliveries.pop_front();
        } else {
            break;
        }
    }
}

void DeliveryManager::clear()
{
    m_pendingDeliveries.clear();
    m_pendingAcks.clear();
}

Delivery::Delivery(uint32 sequenceNumber)
    : sequenceNumber(sequenceNumber)
{
}

Delivery::~Delivery()
{
}
