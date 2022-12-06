#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include <list>
#include <vector>

namespace esphome {
namespace kmp {

enum KmpMeterType { HEAT_METER, POWER_METER };

enum DestinationAddress : unsigned char {
  HEAT_METER = 0x3f,
  LOGGER_TOP = 0x7f,
  LOGGER_BASE = 0xbf,
};

class KmpSensorBase {
 public:
  std::string controller_id;
  uint16_t register_id;
  bool waiting_for_response;
  KmpSensorBase(std::string controller_id, uint16_t register_id);
  virtual void publish_state(float state){};
};

class Kmp : public PollingComponent, public uart::UARTDevice {
 public:
  Kmp(uart::UARTComponent *uart) : uart::UARTDevice(uart) {}
  void register_kmp_sensor(KmpSensorBase *sensor);
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  uint8_t timeout_;
  uint16_t destination_address_;
  std::vector<KmpSensorBase *> kmp_sensors_{};
  float read_register(uint16_t register_id);
  void send_request(char const *msg, int msgsize);
  unsigned short receive_response(char recvmsg[]);
  float decode_reponse(const unsigned int register_id, const char *msg);
  long crc_1021(char const *inmsg, unsigned int len);
};

}  // namespace kmp
}  // namespace esphome
