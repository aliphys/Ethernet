#include "Ethernet.h"
#include <Arduino.h>

#ifdef MOCK_PINS_COUNT
#include "Dns.h"
#include "utility/w5100.h"

std::vector<mockServer> EthernetClient_CI::mockServers;
socket_t EthernetClient_CI::_sockets[MAX_SOCK_NUM];

EthernetClient_CI::EthernetClient_CI() : sockindex(MAX_SOCK_NUM), _timeout(1000) {}

EthernetClient_CI::EthernetClient_CI(uint8_t s) : sockindex(s), _timeout(1000) {
  if (s == MAX_SOCK_NUM) {
    for (int i = 0; i < MAX_SOCK_NUM; ++i) {
      if (_sockets[i].status == SnSR::CLOSED) {
        sockindex = i;
        _status = SnSR::ESTABLISHED;
        break;
      }
    }
  }
}

size_t EthernetClient_CI::write(uint8_t b) { return write(&b, 1); }

size_t EthernetClient_CI::write(const uint8_t *buf, size_t size) {
  if (sockindex < MAX_SOCK_NUM) {
    for (int i = 0; i < size; ++i) {
      _sockets[sockindex].writeBuffer.push_back(buf[i]);
    }
    return size;
  }
  return 0;
}

int EthernetClient_CI::read(uint8_t *buf, size_t bufSize) {
  int size = max(min(bufSize, sockindex < MAX_SOCK_NUM ? _sockets[sockindex].readBuffer.size() : 0), 0);

  for (int i = 0; i < size; ++i) {
    buf[i] = _sockets[sockindex].readBuffer.front();
    _sockets[sockindex].readBuffer.pop_front();
  }
  return size;
}

int EthernetClient_CI::read() {
  if (sockindex >= MAX_SOCK_NUM || _sockets[sockindex].status != SnSR::ESTABLISHED) {
    return -1;
  }
  if (!_sockets[sockindex].readBuffer.empty()) {
    char x = _sockets[sockindex].readBuffer.front();
    _sockets[sockindex].readBuffer.pop_front();
    return x;
  }
  return -1;
}

int EthernetClient_CI::peek() {
  if (sockindex >= MAX_SOCK_NUM || _sockets[sockindex].status != SnSR::ESTABLISHED) {
    return -1;
  }
  if (!_sockets[sockindex].readBuffer.empty()) {
    char x = _sockets[sockindex].readBuffer.front();
    return x;
  }
  return -1;
}

void EthernetClient_CI::stop() {
  if (sockindex < MAX_SOCK_NUM) {
    _sockets[sockindex].readBuffer.clear();
    _sockets[sockindex].writeBuffer.clear();
    _sockets[sockindex].status = SnSR::CLOSED;
    sockindex = MAX_SOCK_NUM;
  }

  // Set port and ip to zero
  _localPort = 0;

  // Close peer connection
  peer.ip = (uint32_t)0;
  peer.port = 0;

  // Close the connection
  _status = SnSR::CLOSED;
}

int EthernetClient_CI::connect(IPAddress ip, uint16_t port) {
  if (sockindex < MAX_SOCK_NUM) {
    if (_sockets[sockindex].status != SnSR::CLOSED) {
      return INVALID_SERVER;  // we are already connected!
    }
  } else {
    for (int i = 0; i < MAX_SOCK_NUM; ++i) {
      if (_sockets[i].status == SnSR::CLOSED) {
        sockindex = i;
        break;
      }
    }
  }
  if (sockindex >= MAX_SOCK_NUM) {
    return INVALID_SERVER;  // unable to obtain a socket!
  }
  // Iterate though vector of mock servers
  for (int i = 0; i < mockServers.size(); ++i) {
    // If we find server with ip and port
    if ((mockServers.at(i).ip == ip) && (mockServers.at(i).port == port)) {
      // Save name, ip, and port in peer
      peer.hostname[0] = '\0';
      peer.ip = ip;
      peer.port = port;
      _status = SnSR::ESTABLISHED;
      _sockets[sockindex].status = SnSR::ESTABLISHED;
      _localPort = 0xC000 + sockindex;
      return SUCCESS;
    }
  }
  return INVALID_SERVER;
}

int EthernetClient_CI::connect(const char *hostname, uint16_t port) {
  if (sockindex < MAX_SOCK_NUM) {
    if (_sockets[sockindex].status != SnSR::CLOSED) {
      return INVALID_SERVER;  // we are already connected!
    }
  } else {
    for (int i = 0; i < MAX_SOCK_NUM; ++i) {
      if (_sockets[i].status == SnSR::CLOSED) {
        sockindex = i;
        break;
      }
    }
  }
  if (sockindex >= MAX_SOCK_NUM) {
    return INVALID_SERVER;  // unable to obtain a socket!
  }
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
      _sockets[sockindex].status = SnSR::ESTABLISHED;
      _localPort = 0xC000 + sockindex;
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
