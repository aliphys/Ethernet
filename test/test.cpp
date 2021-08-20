/*
bundle config --local path vendor/bundle
bundle install
bundle exec arduino_ci_remote.rb --skip-compilation
*/
#include <bitset>
#include <iostream>
#include <vector>

#include "Arduino.h"
#include "ArduinoUnitTests.h"
#include "Ethernet.h"
#include "ci/ObservableDataStream.h"
#include "utility/w5100.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 1, 2};
byte serverIP[] = {192, 168, 1, 1};
const char *serverName = "www.google.com";

unittest_teardown() {
  EthernetClient::stopMockServer(nullptr, serverIP, 80);
  EthernetClient::stopMockServer(serverName, (uint32_t) 0, 80);
  EthernetClient::clearMockServers();
}

unittest(Ethernet_begin_pins) {
  // Setup
  GodmodeState *state = GODMODE();
  state->reset();
  uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  Ethernet.begin(mac);

  // Tests
  int expected[] = {0,    0,  -128, 1, -128, 0,   0, 0, 1,  0,  0,    0, -128,
                    1,    8,  0,    0, 0,    1,   0, 0, 0,  4,  -128, 0, 0,
                    0,    0,  0,    0, 4,    8,   0, 0, 0,  0,  -16,  0, 0,
                    -128, 15, 0,    0, 0,    -16, 0, 0, 16, 15, 0,    0, 0};
  assertEqual(52, state->spi.dataOut.size());
  bool passed = true;
  for (int i = 0; i < 52; i++) {
    if (expected[i] != (int)state->spi.dataOut.at(i)) {
      passed = false;
      break;
    }
  }
  assertTrue(passed);
}

unittest(Ethernet_begin_DHCP) {
  // Setup
  GodmodeState *state = GODMODE();
  state->reset();
  uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  int flag = Ethernet.begin(mac);
  assertEqual(0, flag);  // DHCP failure
  assertEqual(0, Ethernet.localIP());

  Ethernet.mockDHCP(IPAddress(192, 168, 1, 42));
  flag = Ethernet.begin(mac);
  assertEqual(1, flag);  // DHCP success
  assertEqual(IPAddress(192, 168, 1, 42), Ethernet.localIP());
}

unittest(Server_Constructor) {
  // Testing port
  EthernetServer ethernet_server(72);
  assertEqual(72, ethernet_server.getPort());
}

unittest(Server_Begin) {

  EthernetServer server(72);
  assertFalse(server.didCallBegin());

  server.begin();
  assertTrue(server.didCallBegin());
}

unittest(Server_Write) {}

unittest(Server_Available) {}

unittest(Client_Connect_IP) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  int result = client.connect(serverIP, 80);
  assertEqual(INVALID_SERVER, result);
  client.stop();

  EthernetClient::startMockServer(nullptr, serverIP, 80);
  result = client.connect(serverIP, 80);
  assertEqual(SUCCESS, result);
  client.stop();
}

unittest(Client_Connect_Name) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  int result = client.connect(serverName, 80);
  assertEqual(INVALID_SERVER, result);
  client.stop();

  EthernetClient::startMockServer(serverName, (uint32_t) 0, 80);
  result = client.connect(serverName, 80);
  assertEqual(SUCCESS, result);
  client.stop();
}

unittest(Client_Write) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80);
  assertEqual(MAX_SOCK_NUM, client.getSockindex());
  int result = client.connect(serverIP, 80);
  assertEqual(1, result);
  assertEqual(0, client.getSockindex());
  assertNotNull(client.writeBuffer());
  std::deque<uint8_t> buffer = *(client.writeBuffer());
  assertEqual(0, buffer.size());
  assertEqual(0, client.writeBuffer()->size());
  client.write((const uint8_t *)"Hello", 5);
  assertEqual(5, client.writeBuffer()->size());
  assertEqual('H', client.writeBuffer()->at(0));
  client.stop();
}

unittest(Client_Read_data_given_after_connection) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80);
  int result = client.connect(serverIP, 80);
  assertEqual(SUCCESS, result);
  assertEqual(-1, client.read());
  client.pushToReadBuffer('A');
  assertEqual(1, client.available());
  assertEqual('A', client.read());
  assertEqual(0, client.available());
  client.stop();
}

unittest(Client_Read_data_given_before_connection) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80, (const uint8_t *) "ABC");
  int result = client.connect(serverIP, 80);
  assertEqual(SUCCESS, result);
  assertEqual(3, client.available());
  assertEqual('A', client.read());
  assertEqual(2, client.available());
  client.stop();
}

unittest(Client_Stop) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80);
  int result = client.connect(serverIP, 80);
  client.stop();

  assertEqual(-1, client.read());
  assertEqual(0, client.localPort());
  assertEqual(0, client.serverPeer().ip);
  assertEqual(0, client.serverPeer().port);
}

unittest(Client_AvailableForWrite) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80);
  int result = client.connect(serverIP, 80);
  assertNotEqual(1024 * 1023, client.availableForWrite());
  assertEqual(1024 * 1024, client.availableForWrite());
}

unittest(Client_Peek) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(nullptr, serverIP, 80);
  int result = client.connect(serverIP, 80);
  assertEqual(SUCCESS, result);

  assertEqual(-1, client.peek());

  client.pushToReadBuffer('A');
  assertEqual('A', client.peek());
}

unittest(Client_Connected) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  assertFalse(client.connected());

  EthernetClient::startMockServer(nullptr, serverIP, 80);
  int result = client.connect(serverIP, 80);
  assertTrue(client.connected());
}

unittest(Client_Server) {
  EthernetServer ethernet_server(80);
  EthernetClient_CI client1, client2;
  client1 = ethernet_server.accept();
  assertFalse(client1);

  EthernetServer* pServer = EthernetServer::getServerForPort(80);
  assertNotNull(pServer);
  assertEqual(&ethernet_server, pServer);
  pServer->setHasClientCalling(true);
  client1 = ethernet_server.accept();
  assertTrue(client1);
  client2 = ethernet_server.getClient();
  assertTrue(client1 == (const EthernetClient) client2);
}

unittest_main()
