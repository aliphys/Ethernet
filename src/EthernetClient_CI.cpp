#include "Ethernet.h"
#include <Arduino.h>

#ifdef MOCK_PINS_COUNT
#include "Dns.h"
#include "utility/w5100.h"

std::vector<mockServer> EthernetClient_CI::mockServers;
socket_t EthernetClient_CI::_sockets[MAX_SOCK_NUM];

EthernetClient_CI::EthernetClient_CI()
    : sockindex(MAX_SOCK_NUM), _timeout(1000) {}

EthernetClient_CI::EthernetClient_CI(uint8_t s) : sockindex(s), _timeout(1000) {
  if (s == MAX_SOCK_NUM) {
    for (int i = 0; i < MAX_SOCK_NUM; ++i) {
      if (_sockets[i].status == SnSR::CLOSED) {
        sockindex = i;
        setStatus(SnSR::ESTABLISHED);
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
  int available =
      sockindex < MAX_SOCK_NUM ? _sockets[sockindex].readBuffer.size() : 0;
  int size = max(min(bufSize, available), 0);

  for (int i = 0; i < size; ++i) {
    buf[i] = _sockets[sockindex].readBuffer.front();
    _sockets[sockindex].readBuffer.pop_front();
  }
  return size;
}

int EthernetClient_CI::read() {
  if (sockindex >= MAX_SOCK_NUM ||
      _sockets[sockindex].status != SnSR::ESTABLISHED) {
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
  if (sockindex >= MAX_SOCK_NUM ||
      _sockets[sockindex].status != SnSR::ESTABLISHED) {
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
    _sockets[sockindex].status = SnSR::CLOSED;
    sockindex = MAX_SOCK_NUM;
  }
  _localPort = 0;
  peer.ip = (uint32_t)0;
  peer.port = 0;
  setStatus(SnSR::CLOSED);
}

int EthernetClient_CI::connect(const char *hostname, IPAddress ip,
                               uint16_t port) {
  if (sockindex < MAX_SOCK_NUM) {
    if (_sockets[sockindex].status != SnSR::CLOSED) {
      return INVALID_SERVER; // we are already connected!
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
    return INVALID_SERVER; // unable to obtain a socket!
  }
  // Iterate though vector of mock servers
  for (int i = 0; i < mockServers.size(); ++i) {
    // If we find server with hostname or ip and port
    bool flag = hostname ? strncmp(mockServers.at(i).hostname, hostname,
                                   HOSTNAME_SIZE) == 0
                         : false;
    if ((flag || mockServers.at(i).ip == ip) &&
        (mockServers.at(i).port == port)) {
      peer = mockServers.at(i);
      setStatus(SnSR::ESTABLISHED);
      _sockets[sockindex].status = SnSR::ESTABLISHED;
      _localPort = 0xC000 + sockindex;
      if (peer.data) {
        pushToReadBuffer((const char *)peer.data);
      }
      return SUCCESS;
    }
  }
  return INVALID_SERVER;
}

int EthernetClient_CI::connect(IPAddress ip, uint16_t port) {
  return connect(nullptr, ip, port);
}

int EthernetClient_CI::connect(const char *hostname, uint16_t port) {
  return connect(hostname, (uint32_t)0, port);
}

void EthernetClient_CI::setStatus(uint8_t status) {
  if (sockindex < MAX_SOCK_NUM) {
    _sockets[sockindex].status = status;
  }
}

void EthernetClient_CI::startMockServer(const char *hostname, IPAddress ip,
                                        uint16_t port, const uint8_t *data) {
  mockServer server;
  if (hostname) {
    strncpy(server.hostname, hostname, HOSTNAME_SIZE);
  } else {
    server.hostname[0] = '\0';
  }
  server.ip = ip;
  server.port = port;
  server.data = data;
  mockServers.push_back(server);
}

void EthernetClient_CI::stopMockServer(const char *hostname, IPAddress ip,
                                       uint16_t port) {
  for (int i = (mockServers.size() - 1); i >= 0; --i) {
    bool flag = hostname ? strncmp(mockServers.at(i).hostname, hostname,
                                   HOSTNAME_SIZE) == 0
                         : false;
    if ((flag | mockServers.at(i).ip == ip) &&
        (mockServers.at(i).port == port)) {
      mockServers.erase(mockServers.begin() + i);
    }
  }
}

void EthernetClient_CI::pushToReadBuffer(uint8_t value) {
  if (sockindex < MAX_SOCK_NUM) {
    _sockets[sockindex].readBuffer.push_back(value);
  }
}

void EthernetClient_CI::pushToReadBuffer(const char *value) {
  if (sockindex < MAX_SOCK_NUM) {
    for (int i = 0; value[i]; ++i) {
      _sockets[sockindex].readBuffer.push_back(value[i]);
    }
  }
}

std::deque<uint8_t> *EthernetClient_CI::writeBuffer() {
  return sockindex < MAX_SOCK_NUM ? &_sockets[sockindex].writeBuffer : nullptr;
}

#endif
