#pragma once

class ReplicationManagerClient {
public:
    void read(const InputMemoryStream& packet);

	void create(const InputMemoryStream& packet, uint32 networkID) const;
	void update(const InputMemoryStream& packet, uint32 networkID) const;
	void destroy(uint32 networkID) const;
};
