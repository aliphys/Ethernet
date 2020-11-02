#include <bitset>
#include <iostream>
#include <vector>

#include "Arduino.h"
#include "ArduinoUnitTests.h"
#include "Ethernet.h"
#include "ci/ObservableDataStream.h"
#include "utility/w5100.h"

// Test the Ethernet.begin() behaves the same as it did in the simulators
unittest(test_EthernetBegin) {
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

unittest_main()
