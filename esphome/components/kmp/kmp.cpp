#pragma once

#include "kmp.h"

namespace esphome {
namespace kmp {

static const char *const TAG = "kmp";

KmpSensorBase::KmpSensorBase(std::string controller_id, uint16_t register_id)
    : controller_id(std::move(controller_id)), register_id(std::move(register_id)) {}

void Kmp::setup(){
  if (!this->read_meter_type()) {
    this->mark_failed();
  }
};

void Kmp::update() {
  // // check for commands to send
  // if (!command_queue_.empty()) {
  //   ESP_LOGV(TAG, "%zu commands already in queue", command_queue_.size());
  // } else {
  //   ESP_LOGV(TAG, "Updating Kmp component");
  // }

  // for (auto &c : this->command_queue_) {
  //   // ESP_LOGVV(TAG, "Updating range 0x%X", c.start_address);
  //   // update_range_(r);
  // }

  // ESP_LOGV(TAG, "Updating Kmp component");
}

void Kmp::loop() {
  for (auto &sensor : this->kmp_sensors_) {
    if (sensor->request_update && !sensor->waiting_for_data) {
      ESP_LOGVV(TAG, "Updating register %X", sensor->register_id);
      sensor->startUpdate();
      const std::pair<float, std::string> value = this->read_register(sensor->register_id);
      sensor->setData(value.first);
      sensor->setUnit(value.second);
      sensor->completeUpdate();
    }
  }
}

void Kmp::dump_config() {
  ESP_LOGCONFIG(TAG, "KMP:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with meter failed!");
  }
  this->check_uart_settings(1200, 2, uart::UART_CONFIG_PARITY_NONE, 8);
  this->check_uart_settings(2400, 2, uart::UART_CONFIG_PARITY_NONE, 8);
}


bool Kmp::read_meter_type() {
  char recvmsg[20];
  char sendmsg[] = {destination_address_, GetType};
  this->send_request(sendmsg, 2);
  uint16_t rxnum = this->receive_response(recvmsg);

  if (rxnum == 0) {
    return false;
  }

  this->decode_gettype_response(recvmsg);

  return true;
}

std::pair<float, std::string> Kmp::read_register(uint16_t register_id) {
  char buffer[40];  // buffer of bytes to hold the received data
  float value = 0;    // this will hold the final value
  std::string unit = "unknown";
  std::pair rval(value, unit);

  // prepare message to send and send it
  char sendmsg[] = {destination_address_, GetRegister, 0x01, getHigh(register_id), getLow(register_id)};
  this->send_request(sendmsg, 5);

  // listen if we get an answer
  uint16_t rxnum = this->receive_response(buffer);

  // check if number of received bytes > 0
  if (rxnum != 0) {
    // decode the received message
    value = this->decode_getregister_reponse(register_id, buffer);
    unit = getUnit(buffer);
    rval.first = value;
    rval.second = unit;
  }

  return rval;
}

void Kmp::send_request(char const *msg, int msgsize) {
  // append checksum bytes to message
  char newmsg[msgsize + 2];
  for (int i = 0; i < msgsize; i++) {
    newmsg[i] = msg[i];
  }
  newmsg[msgsize++] = 0x00;
  newmsg[msgsize++] = 0x00;
  int c = this->crc_1021(newmsg, msgsize);
  newmsg[msgsize - 2] = getHigh(c);
  newmsg[msgsize - 1] = getLow(c);

  // build final transmit message - escape various bytes
  uint8_t txmsg[20] = {TxSB};  // prefix start byte
  unsigned int txsize = 1;
  for (int i = 0; i < msgsize; i++) {
    uint8_t v = newmsg[i];
    if (v == ACK or v == LF or v == ESC or v == TxSB or v == RxSB) {
      txmsg[txsize++] = ESC;
      txmsg[txsize++] = v ^ FLAG;
    } else {
      txmsg[txsize++] = v;
    }
  }
  txmsg[txsize++] = LF;  // EOL

  // send to serial interface
  this->write_array(txmsg, txsize);
}

uint16_t Kmp::receive_response(char recvmsg[]) {
  char rxdata[50];  // buffer to hold received data
  unsigned long rxindex = 0;
  unsigned long starttime = millis();

  this->flush();  // flush serial buffer - might contain noise

  char r = 0;

  // loop until EOL received or timeout
  while (r != LF) {
    // handle rx timeout
    if (millis() - starttime > receive_timeout_) {
      ESP_LOGW(TAG, "Timed out listening for data");
      return 0;
    }

    // handle incoming data
    if (this->available()) {
      // receive byte
      r = this->read();
      if (r != RxSB) {  // don't append if we see the start marker
        // append data
        rxdata[rxindex] = r;
        rxindex++;
      }
    }
  }

  // remove escape markers from received data
  uint16_t j = 0;
  for (uint16_t i = 0; i < rxindex - 1; i++) {
    if (rxdata[i] == ESC) {
      char v = rxdata[i + 1] ^ FLAG;
      if (v != ACK and v != LF and v != ESC and v != TxSB and v != RxSB) {
        ESP_LOGW(TAG, "Missing escape %X", v);
      }
      recvmsg[j] = v;
      i++;  // skip
    } else {
      recvmsg[j] = rxdata[i];
    }
    j++;
  }

  // check CRC
  if (this->crc_1021(recvmsg, j)) {
    ESP_LOGW(TAG, "CRC error: ");
    return 0;
  }

  return j;
}

void Kmp::decode_gettype_response(const char *msg) {
  // skip if message is not valid
  if (msg[0] != destination_address_ or msg[1] != GetType) {
    ESP_LOGE(TAG, "Got invalid reponse from GetType");
    this->mark_failed();
    return;
  } else {
    uint8_t metertype_high = msg[2];
    uint8_t metertype_low = msg[3];

    uint8_t software_revision_high = msg[4];
    uint8_t software_revision_low = msg[5];

    ESP_LOGI(TAG, "Got meter type %h%h", metertype_high, metertype_low);
    ESP_LOGI(TAG, "Got software revision %c%c", software_revision_high, software_revision_low);
  }
}

float Kmp::decode_getregister_reponse(const unsigned int register_id, const char *msg) {
  // skip if message is not valid
  if (msg[0] != destination_address_ or msg[1] != GetRegister) {
    return false;
  }
  if (msg[2] != getHigh(register_id) or msg[3] != getLow(register_id)) {
    return false;
  }

  // decode the mantissa
  long x = 0;
  for (int i = 0; i < msg[5]; i++) {
    x <<= 8;
    x |= msg[i + 7];
  }

  // decode the exponent
  int i = msg[6] & destination_address_;
  if (msg[6] & RxSB) {
    i = -i;
  };
  float ifl = pow(10, i);
  if (msg[6] & TxSB) {
    ifl = -ifl;
  }

  // return final value
  return (float) (x * ifl);
}

long Kmp::crc_1021(char const *inmsg, unsigned int len) {
  long creg = 0x0000;
  for (unsigned int i = 0; i < len; i++) {
    int mask = TxSB;
    while (mask > 0) {
      creg <<= 1;
      if (inmsg[i] & mask) {
        creg |= 1;
      }
      mask >>= 1;
      if (creg & 0x10000) {
        creg &= 0xffff;
        creg ^= 0x1021;
      }
    }
  }
  return creg;
}

char Kmp::getHigh(char input) { return (input >> 8); }
char Kmp::getLow(char input) { return (input & 0xff); }

std::string Kmp::getUnit(char input[]) {
  return units[input[4]];
}

}  // namespace kmp
}  // namespace esphome
