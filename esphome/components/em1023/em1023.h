#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

using namespace std;

namespace esphome {
namespace em1023 {

class EM1023 : public Component, public PollingComponent {
 public:
  EM1023(uart::UARTComponent *uart, char *decryption_key) {
    serial_ = new uart::UARTDevice(uart);
    decryption_key_ = decryption_key;
  }
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  void set_decryption_key(const std::string &decryption_key);

 private:
  char const *decryption_key_;
  vector<uint8_t> EM1023::HexToBytes(const char *hex_string);

 protected:
  uart::UARTDevice *serial_;
};

}  // namespace em1023
}  // namespace esphome
