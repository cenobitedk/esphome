#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include "ansic1218_transport.h"
#include "ansic1218_identification.h"
#include "ansic1218_logon.h"
#include "ansic1218_logoff.h"
#include "ansic1218_security.h"
#include "ansic1218_read-full.h"
#include "ansic1218_table-01.h"

using namespace std;
using namespace ansic1218;

namespace esphome {
namespace em1023 {

class EM1023Component : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void set_decryption_key(const std::string &decryption_key);

 private:
  std::vector<uint8_t> decryption_key_;
  Transport *transport_{nullptr};

 protected:
  bool trigger_update_;
};

}  // namespace em1023
}  // namespace esphome
