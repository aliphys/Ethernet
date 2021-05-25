#include "Ethernet_CI.h"
#include <Arduino.h>

#ifdef MOCK_PINS_COUNT
#include "Dns.h"
#include "utility/w5100.h"

std::vector<mockServer> EthernetClient_CI::mockServers;
uint16_t EthernetClient_CI::nextPort = 49152; // 49152 to 65535

size_t EthernetClient_CI::write(uint8_t b) { return write(&b, 1); }

size_t EthernetClient_CI::write(const uint8_t *buf, size_t size) {
  for (int i = 0; i < size; ++i) {
    _writeBuffer.push_back(buf[i]);
  }
  return size;
}

int EthernetClient_CI::read(uint8_t *buf, size_t bufSize) {
  int size = max(min(bufSize, _readBuffer.size()), 0);

  for (int i = 0; i < size; ++i) {
    buf[i] = _readBuffer.front();
    _readBuffer.pop_front();
  }
  return size;
}

int EthernetClient_CI::read() {
  if (!_readBuffer.empty()) {
    char x = _readBuffer.front();
    _readBuffer.pop_front();
    return x;
  }
  return -1;
}

int EthernetClient_CI::peek() {
  if (!_readBuffer.empty()) {
    char x = _readBuffer.front();
    return x;
  }
  return -1;
}

void EthernetClient_CI::stop() {

  // Clear read and write buffers
  _readBuffer.clear();
  _writeBuffer.clear();

  // Set port and ip to zero
  _localPort = 0;

  // Close peer connection
  peer.ip = (uint32_t)0;
  peer.port = 0;

  // Close the connection
  _status = SnSR::CLOSED;
}

int EthernetClient_CI::connect(IPAddress ip, uint16_t port) {
  if (++nextPort < 49152) {
    nextPort = 49152;
  }
  _localPort = nextPort;
  // Iterate though vector of mock servers
  for (int i = 0; i < mockServers.size(); ++i) {
    // If we find server with ip and port
    if ((mockServers.at(i).ip == ip) && (mockServers.at(i).port == port)) {
      // Save name, ip, and port in peer
      peer.hostname[0] = '\0';
      peer.ip = ip;
      peer.port = port;
      _status = SnSR::ESTABLISHED;
      return SUCCESS;
    }
  }
  return INVALID_SERVER;
}

int EthernetClient_CI::connect(const char *hostname, uint16_t port) {
  if (++nextPort < 49152) {
    nextPort = 49152;
  }
  _localPort = nextPort;
  // Iterate though vector of mock servers
  for (int i = 0; i < mockServers.size(); ++i) {
    // If we find server with ip and port
    if ((strncmp(mockServers.at(i).hostname, hostname, HOSTNAME_SIZE) == 0) &&
        (mockServers.at(i).port == port)) {
      // Save name, ip, and port in peer
      strncpy(peer.hostname, hostname, HOSTNAME_SIZE);
      peer.ip = {0, 0, 0, 0};
      peer.port = port;
      _status = SnSR::ESTABLISHED;
      return SUCCESS;
    }
  }
  return INVALID_SERVER;
}

void EthernetClient_CI::startMockServer(IPAddress ip, uint16_t port) {
  mockServer server;
  server.hostname[0] = '\0';
  server.ip = ip;
  server.port = port;
  mockServers.push_back(server);
}

void EthernetClient_CI::startMockServer(const char *hostname, uint16_t port) {
  mockServer server;
  strncpy(server.hostname, hostname, HOSTNAME_SIZE);
  server.ip = {0, 0, 0, 0};
  server.port = port;
  mockServers.push_back(server);
}

void EthernetClient_CI::stopMockServer(IPAddress ip, uint16_t port) {
  for (int i = (mockServers.size() - 1); i >= 0; --i) {
    if (mockServers.at(i).ip == ip && mockServers.at(i).port == port) {
      mockServers.erase(mockServers.begin() + i);
    }
  }
}

void EthernetClient_CI::stopMockServer(const char *hostname, uint16_t port) {
  for (int i = (mockServers.size() - 1); i >= 0; --i) {
    if ((strncmp(mockServers.at(i).hostname, hostname, HOSTNAME_SIZE) == 0) &&
        (mockServers.at(i).port == port)) {
      mockServers.erase(mockServers.begin() + i);
    }
  }
}

#endif
