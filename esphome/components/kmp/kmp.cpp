#pragma once

#include "kmp.h"

namespace esphome {
namespace kmp {

static const char *const TAG = "kmp";

KmpSensorBase::KmpSensorBase(std::string controller_id, uint16_t register_id)
    : controller_id(std::move(controller_id)), register_id(std::move(register_id)) {}

void Kmp::register_kmp_sensor(KmpSensorBase *kmp_sensor) { kmp_sensors_.emplace_back(kmp_sensor); }

void Kmp::setup(){

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

  for (auto &c : this->kmp_sensors_) {
  }
}

void Kmp::dump_config() {
  ESP_LOGCONFIG(TAG, "KMP:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with CSE7761 failed!");
  }
  this->check_uart_settings(1200, 2, uart::UART_CONFIG_PARITY_NONE, 8);
  this->check_uart_settings(2400, 2, uart::UART_CONFIG_PARITY_NONE, 8);
}

float Kmp::read_register(uint16_t register_id) {
  char recvmsg[40];  // buffer of bytes to hold the received data
  float rval = 0;    // this will hold the final value

  // prepare message to send and send it
  char sendmsg[] = {HEAT_METER, 0x10, 0x01, static_cast<char>(register_id >> 8), static_cast<char>(register_id & 0xff)};
  this->send_request(sendmsg, 5);

  // listen if we get an answer
  unsigned short rxnum = this->receive_response(recvmsg);

  // check if number of received bytes > 0
  if (rxnum != 0) {
    // decode the received message
    rval = this->decode_reponse(register_id, recvmsg);
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
  newmsg[msgsize - 2] = (c >> 8);
  newmsg[msgsize - 1] = c & 0xff;

  // build final transmit message - escape various bytes
  unsigned char txmsg[20] = {0x80};  // prefix
  unsigned int txsize = 1;
  for (int i = 0; i < msgsize; i++) {
    if (newmsg[i] == 0x06 or newmsg[i] == 0x0d or newmsg[i] == 0x1b or newmsg[i] == 0x40 or newmsg[i] == 0x80) {
      txmsg[txsize++] = 0x1b;
      txmsg[txsize++] = newmsg[i] ^ 0xff;
    } else {
      txmsg[txsize++] = newmsg[i];
    }
  }
  txmsg[txsize++] = 0x0d;  // EOL

  // send to serial interface
  this->write_array(txmsg, txsize);
}

unsigned short Kmp::receive_response(char recvmsg[]) {
  char rxdata[50];  // buffer to hold received data
  unsigned long rxindex = 0;
  unsigned long starttime = millis();

  this->flush();  // flush serial buffer - might contain noise

  char r = 0;

  // loop until EOL received or timeout
  while (r != 0x0d) {
    // handle rx timeout
    if (millis() - starttime > TIMEOUT) {
      // Serial.println("Timed out listening for data");
      ESP_LOGW(TAG, "Timed out listening for data");
      return 0;
    }

    // handle incoming data
    if (this->available()) {
      // receive byte
      r = this->read();
      if (r != 0x40) {  // don't append if we see the start marker
        // append data
        rxdata[rxindex] = r;
        rxindex++;
      }
    }
  }

  // remove escape markers from received data
  unsigned short j = 0;
  for (unsigned short i = 0; i < rxindex - 1; i++) {
    if (rxdata[i] == 0x1b) {
      char v = rxdata[i + 1] ^ 0xff;
      if (v != 0x06 and v != 0x0d and v != 0x1b and v != 0x40 and v != 0x80) {
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

float Kmp::decode_reponse(const unsigned int register_id, const char *msg) {
  // skip if message is not valid
  if (msg[0] != 0x3f or msg[1] != 0x10) {
    return false;
  }
  if (msg[2] != (register_id >> 8) or msg[3] != (register_id & 0xff)) {
    return false;
  }

  // decode the mantissa
  long x = 0;
  for (int i = 0; i < msg[5]; i++) {
    x <<= 8;
    x |= msg[i + 7];
  }

  // decode the exponent
  int i = msg[6] & 0x3f;
  if (msg[6] & 0x40) {
    i = -i;
  };
  float ifl = pow(10, i);
  if (msg[6] & 0x80) {
    ifl = -ifl;
  }

  // return final value
  return (float) (x * ifl);
}

long Kmp::crc_1021(char const *inmsg, unsigned int len) {
  long creg = 0x0000;
  for (unsigned int i = 0; i < len; i++) {
    int mask = 0x80;
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

}  // namespace kmp
}  // namespace esphome
