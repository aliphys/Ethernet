#pragma once
#ifdef MOCK_PINS_COUNT

#include "Client.h"
#include "Server.h"
#include "Udp.h"
#include <Arduino.h>
#include <deque>
#include <set>
#include <vector>
#include <w5100.h>

// Possible return codes from ProcessResponse
// Originally defined in dns.cpp (Why not in header file?)
#define SUCCESS 1
#define TIMED_OUT -1
#define INVALID_SERVER -2
#define TRUNCATED -3
#define INVALID_RESPONSE -4

#define HOSTNAME_SIZE 64
struct mockServer {
  char hostname[HOSTNAME_SIZE];
  IPAddress ip;
  uint16_t port;
};
struct socket_t {
  uint8_t status = SnSR::CLOSED;
  std::deque<uint8_t> readBuffer;
  std::deque<uint8_t> writeBuffer;
};

class Ethernet_CI : public Ethernet_Base {
public:
  // Initialise the Ethernet shield to use the provided MAC address and
  // gain the rest of the configuration through DHCP.
  // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
  static int begin(uint8_t *mac, unsigned long timeout = 60000,
                   unsigned long responseTimeout = 4000);
  static int maintain();
  static EthernetLinkStatus linkStatus();
  static EthernetHardwareStatus hardwareStatus();

  // Manaul configuration
  static void begin(uint8_t *mac, IPAddress ip);
  static void begin(uint8_t *mac, IPAddress ip, IPAddress dns);
  static void begin(uint8_t *mac, IPAddress ip, IPAddress dns,
                    IPAddress gateway);
  static void begin(uint8_t *mac, IPAddress ip, IPAddress dns,
                    IPAddress gateway, IPAddress subnet);
  static void init(uint8_t sspin = 10);

  static void MACAddress(uint8_t *mac_address);
  static IPAddress localIP();
  static IPAddress subnetMask();
  static IPAddress gatewayIP();
  static IPAddress dnsServerIP();

  void setMACAddress(const uint8_t *mac_address);
  void setLocalIP(const IPAddress local_ip);
  void setSubnetMask(const IPAddress subnet);
  void setGatewayIP(const IPAddress gateway);
  void setDnsServerIP(const IPAddress dns_server);
  void setRetransmissionTimeout(uint16_t milliseconds);
  void setRetransmissionCount(uint8_t num);

  static void socketPortRand(uint16_t n);

  // testing
  void mockDHCP(IPAddress ip);

private:
  static IPAddress _mockDHCP;
  static uint8_t socketBegin(uint8_t protocol, uint16_t port);
  static uint8_t socketBeginMulticast(uint8_t protocol, IPAddress ip,
                                      uint16_t port);
  static uint8_t socketStatus(uint8_t s);
  static void socketClose(uint8_t s);
  static void socketConnect(uint8_t s, uint8_t *addr, uint16_t port);
  static void socketDisconnect(uint8_t s);
  static uint8_t socketListen(uint8_t s);
  static uint16_t socketSend(uint8_t s, const uint8_t *buf, uint16_t len);
  static uint16_t socketSendAvailable(uint8_t s);
  static int socketRecv(uint8_t s, uint8_t *buf, int16_t len);
  static uint16_t socketRecvAvailable(uint8_t s);
  static uint8_t socketPeek(uint8_t s);
  static bool socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port);
  static uint16_t socketBufferData(uint8_t s, uint16_t offset,
                                   const uint8_t *buf, uint16_t len);
  static bool socketSendUDP(uint8_t s);

  friend class EthernetClient_Base;
  friend class EthernetServer_Base;
  friend class EthernetUDP;
};

class EthernetClient_CI : public EthernetClient_Base {
public:
  EthernetClient_CI();
  EthernetClient_CI(uint8_t s);

  uint8_t status() { return _status; }
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual int availableForWrite(void) { return 1024 * 1024; } // returns 1mb
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available() { return sockindex < MAX_SOCK_NUM ? _sockets[sockindex].readBuffer.size() : 0; };
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush() {}
  virtual void stop();
  virtual uint8_t connected() { return _status == SnSR::ESTABLISHED; }
  virtual operator bool() { return connected(); }
  virtual bool operator==(const bool value) { return bool() == value; }
  virtual bool operator!=(const bool value) { return bool() != value; }
  virtual bool operator==(const EthernetClient &rhs) const {
    return sockindex == rhs.getSockindex();
  }
  virtual bool operator!=(const EthernetClient &rhs) {
    return !this->operator==(rhs);
  }
  virtual uint16_t localPort() const { return _localPort; };
  virtual IPAddress remoteIP() const { return peer.ip; }
  virtual uint16_t remotePort() const { return peer.port; }

  friend class EthernetServer;

  using Print::write;

  // Testing Support
  static void startMockServer(IPAddress ip, uint16_t port);
  static void startMockServer(const char *host, uint16_t port);
  static void stopMockServer(IPAddress ip, uint16_t port);
  static void stopMockServer(const char *host, uint16_t port);
  virtual std::vector<mockServer> testServers() { return mockServers; }
  virtual mockServer serverPeer() { return peer; }
  void pushToReadBuffer(uint8_t value) { sockindex < MAX_SOCK_NUM ? _sockets[sockindex].readBuffer.push_back(value) : (void) 0; }
  std::deque<uint8_t>* writeBuffer() { return sockindex < MAX_SOCK_NUM ? &_sockets[sockindex].writeBuffer : nullptr; }
  void setStatus(uint8_t status) { _status = status; }
  uint8_t getSockindex() const { return sockindex; }

private:
  uint8_t sockindex; // MAX_SOCK_NUM means client not in use
  uint16_t _timeout;
  uint16_t _localPort = 0;
  uint8_t _status = SnSR::CLOSED;

  static std::vector<mockServer> mockServers;
  mockServer peer;
  static socket_t _sockets[MAX_SOCK_NUM];
};

class EthernetServer_CI : public EthernetServer_Base {
private:
  uint16_t _port;
  bool _didCallBegin = false;
  uint8_t getSocketNumber() const;
  static std::set<EthernetServer_CI *> servers;
  bool hasClientCalling = false;
  EthernetClient_CI client;

public:
  EthernetServer_CI(uint16_t port);
  ~EthernetServer_CI();
  EthernetClient_CI available();
  EthernetClient_CI accept();
  virtual void begin();
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write;

  // Testing Functions
  bool didCallBegin() { return _didCallBegin; }
  uint16_t getPort() { return _port; }
  static EthernetServer_CI *getServerForPort(uint16_t port);
  void setHasClientCalling(bool flag) { hasClientCalling = flag; }
  EthernetClient_CI getClient() { return client; }
};

#endif
