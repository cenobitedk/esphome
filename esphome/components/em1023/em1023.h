#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

using namespace std;

namespace esphome {
namespace em1023 {

class EM1023Component : public Component, public PollingComponent {
 public:
  EM1023Component(char *decryption_key) { decryption_key_ = decryption_key; }

  void set_uart_parent(uart::UARTComponent *parent) { this->serial_ = parent; }

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
