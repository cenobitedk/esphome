#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include "ansic1218_transport.h"

using namespace std;
using namespace ansic1218;

namespace esphome {
namespace em1023 {

class EM1023Component : public PollingComponent {
 public:
  EM1023Component(uart::UARTComponent *uart) { this->serial_ = uart; }
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  void set_decryption_key(const std::string &decryption_key);

 private:
  char const *decryption_key_;
  vector<uint8_t> HexToBytes(const char *hex_string);

 protected:
  uart::UARTComponent *serial_;
};

}  // namespace em1023
}  // namespace esphome
