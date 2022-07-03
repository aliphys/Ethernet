#include "Ethernet.h"
#include <Arduino.h>

#ifdef MOCK_PINS_COUNT

IPAddress Ethernet_CI::_mockDHCP = (uint32_t)0;

int Ethernet_CI::begin(uint8_t *mac, unsigned long timeout,
                       unsigned long responseTimeout) {
  if (_mockDHCP) {
    begin(mac, _mockDHCP);
    return 1;
  }
  return Ethernet_Base::begin(mac, timeout, responseTimeout);
}

int Ethernet_CI::maintain() { return Ethernet_Base::maintain(); }
EthernetLinkStatus Ethernet_CI::linkStatus() {
  return Ethernet_Base::linkStatus();
}
EthernetHardwareStatus Ethernet_CI::hardwareStatus() {
  return Ethernet_Base::hardwareStatus();
}

// Manual configuration
void Ethernet_CI::begin(uint8_t *mac, IPAddress ip) {
  return Ethernet_Base::begin(mac, ip);
}
void Ethernet_CI::begin(uint8_t *mac, IPAddress ip, IPAddress dns) {
  return Ethernet_Base::begin(mac, ip, dns);
}
void Ethernet_CI::begin(uint8_t *mac, IPAddress ip, IPAddress dns,
                        IPAddress gateway) {
  return Ethernet_Base::begin(mac, ip, dns, gateway);
}
void Ethernet_CI::begin(uint8_t *mac, IPAddress ip, IPAddress dns,
                        IPAddress gateway, IPAddress subnet) {
  return Ethernet_Base::begin(mac, ip, dns, gateway, subnet);
}
void Ethernet_CI::init(uint8_t sspin) { return Ethernet_Base::init(sspin); }

void Ethernet_CI::MACAddress(uint8_t *mac_address) {
  return Ethernet_Base::MACAddress(mac_address);
}
IPAddress Ethernet_CI::localIP() {
  return _mockDHCP ? _mockDHCP : Ethernet_Base::localIP();
}
IPAddress Ethernet_CI::subnetMask() { return Ethernet_Base::subnetMask(); }
IPAddress Ethernet_CI::gatewayIP() { return Ethernet_Base::gatewayIP(); }
IPAddress Ethernet_CI::dnsServerIP() { return Ethernet_Base::dnsServerIP(); }

void Ethernet_CI::setMACAddress(const uint8_t *mac_address) {
  Ethernet_Base::setMACAddress(mac_address);
}
void Ethernet_CI::setLocalIP(const IPAddress local_ip) {
  Ethernet_Base::setLocalIP(local_ip);
}
void Ethernet_CI::setSubnetMask(const IPAddress subnet) {
  Ethernet_Base::setSubnetMask(subnet);
}
void Ethernet_CI::setGatewayIP(const IPAddress gateway) {
  Ethernet_Base::setGatewayIP(gateway);
}
void Ethernet_CI::setDnsServerIP(const IPAddress dns_server) {
  Ethernet_Base::setDnsServerIP(dns_server);
}
void Ethernet_CI::setRetransmissionTimeout(uint16_t milliseconds) {
  Ethernet_Base::setRetransmissionTimeout(milliseconds);
}
void Ethernet_CI::setRetransmissionCount(uint8_t num) {
  Ethernet_Base::setRetransmissionCount(num);
}

// testing support
void Ethernet_CI::mockDHCP(IPAddress ip) { _mockDHCP = ip; }

Ethernet_CI Ethernet;

#endif // MOCK_PINS_COUNT
