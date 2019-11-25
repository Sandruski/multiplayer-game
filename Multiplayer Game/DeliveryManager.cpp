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

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 sequenceNumber;
	packet.Read(sequenceNumber);
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
	for (uint32 i = 0; i < size; ++i)
	{
        packet.Write(m_pendingAcks[i]);
    }

	m_pendingAcks.clear();
}

void DeliveryManager::processAckSequenceNumbers(const InputMemoryStream& packet)
{
    std::size_t size;
    packet.Read(size);
	for (uint32 i = 0; i < size; ++i)
	{
		uint32 sequenceNumber;
		packet.Read(sequenceNumber);

		for (auto it = m_pendingDeliveries.begin(); it != m_pendingDeliveries.end(); ++it)
		{
			Delivery* delivery = *it;
			if (delivery->sequenceNumber == sequenceNumber)
			{
				delivery->delegate->onDeliverySuccess(this);

				RELEASE(delivery);
				m_pendingDeliveries.erase(it);
				break;
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
    float timeout = static_cast<float>(Time.time) - ACK_INTERVAL_SECONDS;
	for (auto it = m_pendingDeliveries.begin(); it != m_pendingDeliveries.end(); ++it)
	{
		Delivery* delivery = *it;
        if (delivery->dispatchTime < timeout) 
		{
            delivery->delegate->onDeliveryFailure(this);

			RELEASE(delivery);
			it = m_pendingDeliveries.erase(it);
			continue;
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

	m_nextOutgoingSequenceNumber = 0;
	m_nextExpectedSequenceNumber = 0;
}

Delivery::Delivery(uint32 sequenceNumber)
    : sequenceNumber(sequenceNumber)
{
}

Delivery::~Delivery()
{
	RELEASE(delegate);
}
