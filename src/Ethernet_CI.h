#pragma once
// Needed for workaround for problem in Arduino.CI
#include <IPAddress.h>
#include <Ethernet.h>
#ifdef MOCK_PINS_COUNT

// Configure the maximum number of sockets to support.  W5100 chips can have
// up to 4 sockets.  W5200 & W5500 can have up to 8 sockets.  Several bytes
// of RAM are used for each socket.  Reducing the maximum can save RAM, but
// you are limited to fewer simultaneous connections.
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define MAX_SOCK_NUM 4
#else
#define MAX_SOCK_NUM 8
#endif

#include "Client.h"
#include "Server.h"
#include "Udp.h"
#include <w5100.h>
#include <Arduino.h>
#include <vector>
#include <deque>

// Possible return codes from ProcessResponse
// Originally defined in dns.cpp (Why not in header file?)
#define SUCCESS          1
#define TIMED_OUT        -1
#define INVALID_SERVER   -2
#define TRUNCATED        -3
#define INVALID_RESPONSE -4

struct mockServer {
  IPAddress ip;
  uint16_t port;
};

class EthernetClient_CI : public EthernetClient_Base {
public:
  EthernetClient_CI() : sockindex(MAX_SOCK_NUM), _timeout(1000) { }
  EthernetClient_CI(uint8_t s) : sockindex(s), _timeout(1000) { }

  uint8_t status() { return _status; }
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port) { return INVALID_SERVER; }
  virtual int availableForWrite(void) { return 1024 * 1024; } // returns 1mb
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available() { return _writeBuffer.size(); };
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush() {}
  virtual void stop();
  virtual uint8_t connected() { return _status == SnSR::ESTABLISHED; }
  virtual operator bool() { return connected(); }
  virtual bool operator==(const bool value) { return bool() == value; }
  virtual bool operator!=(const bool value) { return bool() != value; }
  virtual bool operator==(const EthernetClient& rhs) { return _localPort == rhs.localPort(); }
  virtual bool operator!=(const EthernetClient& rhs)
     { return !this->operator==(rhs); }
  virtual uint16_t localPort() const { return _localPort; };

  virtual IPAddress remoteIP() const { return peer.ip; }
  virtual uint16_t remotePort() const { return peer.port; }
  virtual std::vector<mockServer> testServers() { return mockServers; }
  virtual mockServer serverPeer() { return peer; }

  friend class EthernetServer;

  using Print::write;

  // Testing Support
  static void startMockServer(IPAddress ip, uint16_t port);
  static void stopMockServer(IPAddress ip, uint16_t port);
  void pushToReadBuffer(uint8_t value) { _readBuffer.push_back(value); }
  std::deque<uint8_t> writeBuffer() {return _writeBuffer;}


private:
  uint8_t sockindex; // MAX_SOCK_NUM means client not in use
  uint16_t _timeout;
  static std::vector<mockServer> mockServers;
  static uint16_t nextPort; // 49152 to 65535
  mockServer peer;
  uint16_t _localPort = 0;
  uint8_t _status = SnSR::CLOSED;
  std::deque<uint8_t> _readBuffer, _writeBuffer;
};

class EthernetServer_CI : public EthernetServer_Base {
private:
  uint16_t _port;
  bool _didCallBegin = false;
  uint8_t getSocketNumber() const;

public:
  EthernetServer_CI(uint16_t port) : EthernetServer_Base(port) { _port = port; }
  EthernetClient_CI available();
  EthernetClient_CI accept();
  virtual void begin();
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write;

  // Testing Functions
  bool didCallBegin() { return _didCallBegin; }
  uint16_t getPort() { return _port; }
};

#endif