#include <Arduino.h>
#include "Ethernet.h"

#ifdef MOCK_PINS_COUNT
#include "utility/w5100.h"

std::set<EthernetServer_CI*> EthernetServer_CI::servers;

EthernetServer_CI::EthernetServer_CI(uint16_t port) : EthernetServer_Base(port) {
	_port = port;
	servers.emplace(this);
}

EthernetServer_CI::~EthernetServer_CI() {
	servers.erase(this);
}

void EthernetServer_CI::begin()
{
	_didCallBegin = true;
}

EthernetClient_CI EthernetServer_CI::available()
{
	return accept();
}

EthernetClient_CI EthernetServer_CI::accept()
{
	if (hasClientCalling) {
		client = EthernetClient_CI(MAX_SOCK_NUM);  // signal to select your own sockindex and "connect"
	} else {
		client = EthernetClient_CI();
	}
	return client;
}

size_t EthernetServer_CI::write(const uint8_t *buffer, size_t size)
{
	EthernetClient_CI client = EthernetClient_CI(getSocketNumber());
	if (!client) {  // test if client is connected
		return 0;
	}
	return client.write(buffer, size);
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

#endif
