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
#include "Ethernet_CI.h"
#include "ci/ObservableDataStream.h"
#include "utility/w5100.h"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10, 0, 0, 177 };
byte server[] = { 64, 233, 187, 99 }; // Google

// Test the Ethernet.begin() behaves the same as it did in the simulators
unittest(EthernetBegin) {
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

unittest(Server_Write) {

}

unittest(Server_Available) {

}

unittest(Client_Connect) {

  Ethernet.begin(mac, ip);
  EthernetClient client;
  int result = client.connect(server, 80);
  assertEqual(INVALID_SERVER,result);
  client.stop();

  EthernetClient::startMockServer(server,80);
  result = client.connect(server, 80);
  assertEqual(SUCCESS,result);
  client.stop();
  EthernetClient::stopMockServer(server,80);
}

unittest(Client_Write) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  assertEqual(0,client.writeBuffer().size());
  client.write((const uint8_t*)"Hello",5);
  assertEqual(5,client.writeBuffer().size());
  assertEqual('H',client.writeBuffer().at(0));
  EthernetClient::stopMockServer(server,80);
}

unittest(Client_Read) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  assertEqual(SUCCESS,result);
  assertEqual(-1,client.read());
  client.pushToReadBuffer('A');
  assertEqual('A',client.read());
  EthernetClient::stopMockServer(server,80);
}

unittest(Client_Stop) {
  Ethernet.begin(mac, ip);
  EthernetClient client;
  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  client.stop();

  assertEqual(-1, client.read());
  assertEqual(0,client.writeBuffer().size());

  assertEqual(0,client.localPort());

  assertEqual(0,client.serverPeer().ip);
  assertEqual(0,client.serverPeer().port);
  EthernetClient::stopMockServer(server,80);
}

unittest(Client_AvailableForWrite) {
  Ethernet.begin(mac,ip);
  EthernetClient client;
  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  assertNotEqual(1024*1023,client.availableForWrite());
  assertEqual(1024*1024,client.availableForWrite());
}

unittest(Client_Peek) {
  Ethernet.begin(mac,ip);
  EthernetClient client;
  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  assertEqual(SUCCESS,result);

  assertEqual(-1,client.peek());

  client.pushToReadBuffer('A');
  assertEqual('A',client.peek());
}

unittest(Client_Connected) {
  Ethernet.begin(mac,ip);
  EthernetClient client;
  assertFalse(client.connected());

  EthernetClient::startMockServer(server,80);
  int result = client.connect(server, 80);
  assertTrue(client.connected());
}

unittest_main()
