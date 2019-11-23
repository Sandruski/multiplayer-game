#pragma once

class DeliveryManager;

class DeliveryDelegate {
public:
    virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
    virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery {
    Delivery(uint32 sequenceNumber);
    ~Delivery();

    uint32 sequenceNumber = 0;
    double dispatchTime = 0.0;
    DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager {
public:
	DeliveryManager();
	~DeliveryManager();

    // For senders to write a new sequence number into a packet
    Delivery* writeSequenceNumber(OutputMemoryStream& packet);
    // For receivers to process the sequence number from an incoming packet
    bool processSequenceNumber(uint32 sequenceNumber);

    // For receivers to write ack'ed sequence numbers into a packet
    bool hasSequenceNumbersPendingAck() const;
    void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

    // For senders to process ack'ed numbers from a packet
    void processAckSequenceNumbers(const InputMemoryStream& packet);
    void processTimedOutPackets();

    void clear();

private:
    // Sender side
    uint32 m_nextOutgoingSequenceNumber = 0;
    std::deque<Delivery*> m_pendingDeliveries;

    // Receiver side
    uint32 m_nextExpectedSequenceNumber = 0;
    std::deque<uint32> m_pendingAcks;
};
