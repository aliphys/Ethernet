#include <Arduino.h>
#include "Ethernet.h"

#ifdef MOCK_PINS_COUNT
#include "utility/w5100.h"

std::vector<EthernetServer_CI*> EthernetServer_CI::servers;

EthernetServer_CI::EthernetServer_CI(uint16_t port) : EthernetServer_Base(port) {
	_port = port;
	servers.push_back(this);
}

void EthernetServer_CI::begin()
{
	// EthernetServer_Base::begin();
	_didCallBegin = true;
}

EthernetClient_CI EthernetServer_CI::available()
{
	EthernetClient_Base client = EthernetServer_Base::available();
	return EthernetClient_CI(client.getSocketNumber());
}

EthernetClient_CI EthernetServer_CI::accept()
{
	EthernetClient_Base client = EthernetServer_Base::accept();
	return EthernetClient_CI(client.getSocketNumber());
}

size_t EthernetServer_CI::write(const uint8_t *buffer, size_t size)
{
	EthernetClient_CI client = EthernetClient_CI(getSocketNumber());

	// Is this necessary?
	if (!client) {
		return 0;
	}
	return client.write(buffer,size);
}

uint8_t EthernetServer_CI::getSocketNumber() const {
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		if (server_port[i] == _port) {
			return i;
		}
	}
	return MAX_SOCK_NUM;
}

EthernetServer_CI* EthernetServer_CI::getServerForPort(uint16_t port) {
	for (auto each : servers) {
		if (each->getPort() == port) {
			return each;
		}
	}
	return nullptr;
}

void EthernetServer_CI::setNextClient(EthernetClient_CI nextClient) {
	this->nextClient = nextClient;
}

#endif
