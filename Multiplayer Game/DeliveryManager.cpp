#include "DeliveryManager.h"
#include "Networks.h"

DeliveryManager::DeliveryManager()
{
}

DeliveryManager::~DeliveryManager()
{
	clear();
}

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
    std::size_t size = m_pendingAcks.size();
    packet.Write(size);
    if (size > 0) {
        uint32 firstSequenceNumber = m_pendingAcks.front();
        packet.Write(firstSequenceNumber);
    }
}

void DeliveryManager::processAckSequenceNumbers(const InputMemoryStream& packet)
{
    std::size_t size;
    packet.Read(size);
    if (size > 0) {
        uint32 firstSequenceNumber;
        packet.Read(firstSequenceNumber);

        uint32 lastSequenceNumber = firstSequenceNumber + static_cast<uint32>(size) - 1;
        while (firstSequenceNumber < lastSequenceNumber
            && !m_pendingDeliveries.empty()) {
            Delivery* delivery = m_pendingDeliveries.front();
            uint32 deliverySequenceNumber = delivery->sequenceNumber;
            if (deliverySequenceNumber >= m_nextExpectedSequenceNumber) {
                delivery->delegate->onDeliverySuccess(this);
                m_nextExpectedSequenceNumber = deliverySequenceNumber + 1;
            } else {
                delivery->delegate->onDeliveryFailure(this);
            }
			RELEASE(delivery);
            m_pendingDeliveries.pop_front();
        }
    }
}

void DeliveryManager::processTimedOutPackets()
{
    float timeout = static_cast<float>(Time.time) - ACK_INTERVAL_SECONDS;
    while (!m_pendingDeliveries.empty()) {
        Delivery* delivery = m_pendingDeliveries.front();
        if (delivery->dispatchTime < timeout) {
            delivery->delegate->onDeliveryFailure(this);
			RELEASE(delivery);
            m_pendingDeliveries.pop_front();
        } else {
            break;
        }
    }
}

void DeliveryManager::clear()
{
	while (!m_pendingDeliveries.empty())
	{
		RELEASE(m_pendingDeliveries.front());
		m_pendingDeliveries.pop_front();
	}

    m_pendingDeliveries.clear();
    m_pendingAcks.clear();
}

Delivery::Delivery(uint32 sequenceNumber)
    : sequenceNumber(sequenceNumber)
{
}

Delivery::~Delivery()
{
	RELEASE(delegate);
}
