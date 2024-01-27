// Copyright 2021 Time Energy Industria e Comercio Ltda
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef USE_ESP32

#include <algorithm>
#include <cassert>
#include <iomanip>
// #include <endian.h>
// #include <machine/endian.h>
#include "esphome/core/log.h"
#include "ansic1218_transport.h"

using namespace std;
using namespace esphome;
using namespace ansic1218;
using namespace service;

static const char *TAG = "ansic1218::transport";

struct Transport::Packet {
  enum { START_OF_PACKET = 0xee, IDENTITY = 0x05 };

  uint8_t stp;
  uint8_t identity;
  uint8_t ctrl;
  uint8_t seq_nbr;
  uint16_t length;

} __attribute__((__packed__));

// Transport::Transport(uart::UARTDevice *serial) : serial(move(serial)) {
Transport::Transport(uart::UARTComponent *uart_ptr) {
  // Prepare serial.
  esphome::uart::UARTDevice serial(uart_ptr);

  transport_mutex = xSemaphoreCreateMutex();
  if (!transport_mutex)
    ESP_LOGE(TAG, "Failed to create transport mutex.");
  // throw Exception(__PRETTY_FUNCTION__, "Failed to create transport mutex.");
}

bool Transport::request(service::Service &&service) { return request(service); }

bool Transport::request(service::Service &service) {
  if (xSemaphoreTake(transport_mutex, CONFIG_ANSI_TRANSPORT_MUTEX_BLOCKTIME / portTICK_PERIOD_MS) != pdTRUE) {
    ESP_LOGW(TAG, "Timeout to take mutex");
    return false;
  }

  ESP_LOGD(TAG, "Creating packet");

  Packet packet{.stp = Packet::START_OF_PACKET, .identity = Packet::IDENTITY, .ctrl = 0, .seq_nbr = 0, .length = 0};

  const uint8_t MULTI_PACKET = 0b10000000;
  const uint8_t FIRST_PACKET = 0b01000000;

  ESP_LOGD(TAG, "Send {ACK}");

  vector<uint8_t> data{ACK};
  send(data);

  ESP_LOGD(TAG, "Assertion");

  static_assert(sizeof(packet) == 6, "");

  auto *ptr = reinterpret_cast<uint8_t *>(&packet);

  vector<uint8_t> sent{ptr, &ptr[sizeof(packet)]};

  ESP_LOGD(TAG, "Build request frame");

  if (!service.request(sent)) {
    ESP_LOGW(TAG, "Could not properly build request frame");
    xSemaphoreGive(transport_mutex);
    return false;
  }

  auto *p_packet = reinterpret_cast<Packet *>(sent.data());
  p_packet->length = convert_big_endian(sent.size() - sizeof(Packet));

  ESP_LOGD(TAG, "CRC calculate");

  CRC::calculate(sent.cbegin(), sent.cend(), sent);

  ESP_LOGD(TAG, "Send request");
  send(sent);

  vector<uint8_t> content;
  uint8_t max_request = 0;
  int seq_nbr = 0;

  vector<uint8_t> received;

  ESP_LOGD(TAG, "Wait for response");

  bool ret = wait(received, {ACK});

  while (!ret && max_request < 3) {
    max_request++;
    ESP_LOGD(TAG, "Sending packet again. Requested: %d times", int(max_request));
    flush();
    send(sent);
    received.clear();
    ret = wait(received, {ACK});
  }

  if (!ret) {
    ESP_LOGD(TAG, "Got no response ");
    ret = nack(service, sent, received, "ACK not received.");
    xSemaphoreGive(transport_mutex);
    return ret;
  }

  ESP_LOGD(TAG, "Response received, start processing ...");

  do {
    received.clear();

    if (!receive(received, sizeof(packet))) {
      ret = nack(service, sent, received, "failed to receive header.");
      xSemaphoreGive(transport_mutex);
      return ret;
    }

    ESP_LOGD(TAG, "Received:");
    ESP_LOG_BUFFER_HEX(TAG, received.data(), received.size());
    // ESP_LOGD(TAG, "Received: %X %d", received.data(), received.size());
    // ESP_LOG_BUFFER_HEX_LEVEL(TAG, received.data(), received.size(), ESP_LOG_DEBUG);
    p_packet = reinterpret_cast<Packet *>(received.data() + received.size() - sizeof(packet));
    if (!validate(p_packet)) {
      ret = nack(service, sent, received, "received an invalid header");
      xSemaphoreGive(transport_mutex);
      return ret;
    }

    seq_nbr = int(p_packet->seq_nbr);

    if (p_packet->ctrl & MULTI_PACKET) {
      ESP_LOGD(TAG, "Received a multi-packet response.");
      ESP_LOGD(TAG, "Sequence number of the packet is: %d", seq_nbr);
      if (p_packet->ctrl & FIRST_PACKET)
        ESP_LOGD(TAG, "First frame of multi-packet response.");
    }

    if (!receive(received, convert_big_endian(p_packet->length))) {
      ret = nack(service, sent, received, "failed to receive response.");
      xSemaphoreGive(transport_mutex);
      return ret;
    }

    auto crc = CRC::calculate(received.cbegin(), received.cend(), {});

    if (!wait(received, crc)) {
      ret = nack(service, sent, received, "failed to receive CRC");
      xSemaphoreGive(transport_mutex);
      return ret;
    }

    send({ACK});

    copy(received.cbegin() + sizeof(Packet), received.cend() - crc.size(), back_inserter(content));

  } while (seq_nbr > 0);

  ret = service.response(content.cbegin(), content.cend());
  xSemaphoreGive(transport_mutex);
  return ret;
}

void Transport::send(const vector<uint8_t> data) {
  serial.write_array(data);
  ESP_LOGD(TAG, "send(): %s .", bufToStr(data.cbegin(), data.cend()).c_str());
}

bool Transport::wait(std::vector<uint8_t> &buffer, const vector<uint8_t> &data) {
  auto nBytesRead = receive(buffer, data.size());

  if (nBytesRead <= 0)
    return false;

  if (!equal(data.cbegin(), data.cend(), buffer.cend() - nBytesRead)) {
    ESP_LOGW(TAG, "Wait()  expected: %s, received: %s .", bufToStr(data.cbegin(), data.cend()).c_str(),
             bufToStr(buffer.cend() - nBytesRead, buffer.cend()).c_str());
    return false;
  }

  return true;
}

int Transport::receive(vector<uint8_t> &buffer, size_t size) {
  // auto nBytesRead = serial->read(buffer, size, seconds(2));
  auto nBytesRead = serialRead(buffer, size);

  if (nBytesRead != size) {
    ESP_LOGD(TAG, "Receive() received less bytes than expected. expected: %d, received: %d", size, nBytesRead);
    return false;
  }

  ESP_LOGD(TAG, "received(): %s .", bufToStr(buffer.cbegin(), buffer.cend()).c_str());

  return nBytesRead;
}

int Transport::serialRead(vector<uint8_t> &buffer, size_t size) {
  // convert buffer vector to array
  uint8_t *buffer_array = &buffer[0];
  // read to array
  serial.read_array(buffer_array, size);
  // get length
  // auto nBytesRead = (sizeof(buffer_array) / sizeof(*buffer_array));
  auto nBytesRead = sizeof(buffer_array);

  // insert into vector buffer
  buffer.insert(buffer.begin(), buffer_array, buffer_array + nBytesRead);

  return nBytesRead;
}

void Transport::flush() {
  // for (vector<uint8_t> buffer; 0 < serial->read(buffer, 100, milliseconds(100));) {
  for (vector<uint8_t> buffer; 0 < serialRead(buffer, 100);) {
    ESP_LOGD(TAG, "Flushed:  %s", bufToStr(buffer.cbegin(), buffer.cend()).c_str());
    // ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer.data(), buffer.size(), ESP_LOG_DEBUG);
    buffer.clear();
  }
}

bool Transport::nack(const vector<uint8_t> &sent, const vector<uint8_t> &received) {
  ESP_LOGD(TAG, "Sent:     %s", bufToStr(sent.cbegin(), sent.cend()).c_str());
  ESP_LOGD(TAG, "Received: %s", bufToStr(received.cbegin(), received.cend()).c_str());

  flush();

  send({ACK});
  return false;
}

bool Transport::validate(Packet *packet) { return packet->stp == Packet::START_OF_PACKET; }

std::string Transport::bufToStr(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  ostringstream ss;
  ss << "{ ";
  for_each(first, last, [&ss](uint8_t value) { ss << setfill('0') << setw(2) << hex << int(value) << " "; });
  ss << "}";
  return ss.str();
}

#endif
