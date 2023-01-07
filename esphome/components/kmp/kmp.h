#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

// #include <map>
// #include <list>
#include <vector>

namespace esphome {
namespace kmp {

enum DestinationAddress : unsigned char {
  HEAT_METER = 0x3f,
  LOGGER_TOP = 0x7f,
  LOGGER_BASE = 0xbf,
};

enum ReservedBytes : unsigned char {
  TxSB = 0x80,    // €, Request Start Byte
  RxSB = 0x40,    // @, Message Start Byte
  ESC = 0x1b,     // Escape character
  LF = 0x0d,      // Line Feed, Stop Byte
  ACK = 0x06,     // Acknowledge
};

enum StuffingBytes : unsigned char {
  FLAG = 0xff,    // Flag for byte stuffing
};

enum CommandID : unsigned char {
  GetType = 0x01,
  GetSerialNo = 0x02,
  GetRegister = 0x10, // Must be followed by a number of registers to read.
};

const std::string units[] = {
  "", "Wh", "kWh", "MWh", "GWh", "j", "kj", "Mj", "Gj", "Cal", "kCal",
  "Mcal", "Gcal", "varh", "kvarh", "Mvarh", "Gvarh", "VAh", "kVAh", "MVAh",
  "GVAh", "kW", "kW", "MW", "GW", "kvar", "kvar", "Mvar", "Gvar", "VA",
  "kVA", "MVA", "GVA", "V", "A", "kV", "kA", "C", "K", "l", "m3", "l/h",
  "m3/h", "m3xC", "ton", "ton/h", "h", "hh:mm:ss", "yy:mm:dd", "yyyy:mm:dd",
  "mm:dd", "", "bar", "RTC", "ASCII", "m3 x 10", "ton xr 10", "GJ x 10",
  "minutes", "Bitfield", "s", "ms", "days", "RTC-Q", "Datetime"
};

class KmpSensorBase {
 public:
  std::string controller_id;
  uint16_t register_id;
  bool request_update;
  bool waiting_for_data;

  KmpSensorBase(std::string controller_id, uint16_t register_id);

  void startUpdate() {
    waiting_for_data = true;
    request_update = false;
  }
  void completeUpdate() {
    waiting_for_data = false;
    this->clearData();
  }
  void setData(float value) {
    data_to_be_published_ = value;
    has_data_ = true;
  }
  void clearData() {
    data_to_be_published_ = float();
    has_data_ = false;
  }
  void setUnit(std::string unit) { unit_ = unit; }

 protected:
  float data_to_be_published_;
  bool has_data_;
  std::string unit_;
};

class Kmp : public PollingComponent, public uart::UARTDevice {
 public:
  Kmp(uart::UARTComponent *uart) : uart::UARTDevice(uart) {}
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

  void set_destination_address(uint8_t destination_address) { this->destination_address_ = destination_address; }
  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }
  void register_sensor(KmpSensorBase *sensor) { kmp_sensors_.emplace_back(sensor); }

 protected:
  uint8_t destination_address_;
  uint32_t receive_timeout_;
  std::vector<KmpSensorBase *> kmp_sensors_{};

  bool read_meter_type();
  std::pair<float, std::string> read_register(uint16_t register_id);
  void send_request(char const *msg, int msgsize);
  uint16_t receive_response(char recvmsg[]);
  void decode_gettype_response(const char *msg);
  float decode_getregister_reponse(const unsigned int register_id, const char *msg);
  long crc_1021(char const *inmsg, unsigned int len);

  char getHigh(char input);
  char getLow(char input);
  std::string getUnit(char input[]);
};

}  // namespace kmp
}  // namespace esphome
