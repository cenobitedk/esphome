#include "em1023.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include "ansic1218_transport.h"
#include "ansic1218_identification.h"
#include "ansic1218_security.h"
#include "ansic1218_read-full.h"
#include "ansic1218_table-01.h"

using namespace ansic1218;
using namespace ansic1218::service;
using namespace ansic1218::table;

namespace esphome {
namespace em1023 {

static const char *const TAG = "em1023";

void EM1023Component::setup() {
  // auto serial = make_shared<uart::UARTComponent>(this->serial_);

  // Transport transport_(this->serial_);

  // Transport *transport_ = new Transport(this->serial_);
  Transport *transport_ = new Transport(this->uart_ptr_);
}

void EM1023Component::update() { this->trigger_update_ = true; }

void EM1023Component::loop() {
  if (this->trigger_update_) {
    ESP_LOGI(TAG, "update() start");
    this->trigger_update_ = false;

    // uint32_t result;
    // if (this->read_sensor_(&result)) {
    //   int32_t value = static_cast<int32_t>(result);
    //   ESP_LOGD(TAG, "'%s': Got value %d", this->name_.c_str(), value);
    //   this->publish_state(value);
    // }

    // vector<uint8_t> identity{};

    ESP_LOGD(TAG, "Key: %s", this->decryption_key_);
    vector<uint8_t> raw_pass = HexToBytes(this->decryption_key_);

    ESP_LOGD(TAG, "Key: %s", raw_pass.data());

    // auto serial = make_shared<uart::UARTComponent>(this->serial_);

    // Transport transport(this->serial_);

    // Identification *identification();
    Identification *identification = new Identification();

    if (!this->transport_.request(*identification)) {
      ESP_LOGE(TAG, "Could not request identity service");
      return;
    }

    // identity = identification.getDeviceIdentity();

    // if (!transport.request(Security(identity, raw_pass))) {
    //   ESP_LOGE(TAG, "could not request security service");
    //   return;
    // }

    // Table01 table01;

    // if (!transport.request(ReadFull(table01)))
    //   return;

    // auto info_01 = table01.content();
    // string meter_serial(info_01->mfg_serial_number, sizeof(info_01->mfg_serial_number));
    // ostringstream ofw;
    // ostringstream ohw;

    // ofw << (int) info_01->fw_version_number << "." << (int) info_01->fw_revision_number;
    // ohw << (int) info_01->hw_version_number << "." << (int) info_01->hw_revision_number;

    // ESP_LOGI(TAG, "Meter firmware: %s", ofw.str().c_str());
    // ESP_LOGI(TAG, "Meter hardware: %s", ohw.str().c_str());
    // ESP_LOGI(TAG, "Meter serial: %s", meter_serial.c_str());

    ESP_LOGI(TAG, "update() done");
  }
}

void EM1023Component::dump_config() {
  ESP_LOGCONFIG(TAG, "config:");
  // LOG_SENSOR("", "EM1023", this);
  // LOG_PIN("  DOUT Pin: ", this->dout_pin_);
  // LOG_PIN("  SCK Pin: ", this->sck_pin_);

  // this->serial_->check_uart_settings()

  // LOG_UPDATE_INTERVAL(this);
}

float EM1023Component::get_setup_priority() const { return setup_priority::LATE; }

void EM1023Component::set_decryption_key(const std::string &decryption_key) {
  if (decryption_key.length() == 0) {
    ESP_LOGI(TAG, "Disabling decryption");
    this->decryption_key_ = nullptr;
    // this->decryption_key_.clear();
    if (this->decryption_key_ != nullptr) {
      delete[] this->decryption_key_;
      this->decryption_key_ = nullptr;
    }
    return;
  }

  // if (decryption_key.length() != 32) {
  //   ESP_LOGE(TAG, "Error, decryption key must be 32 character long");
  //   return;
  // }

  this->decryption_key_ = nullptr;
  // this->password_.clear();

  ESP_LOGI(TAG, "Decryption key is set");
  // Verbose level prints decryption key
  ESP_LOGD(TAG, "Using decryption key: %s", decryption_key.c_str());

  this->decryption_key_ = decryption_key.c_str();
}

vector<uint8_t> EM1023Component::HexToBytes(const char *hex_string) {
  string hex(hex_string);
  vector<uint8_t> bytes;

  for (unsigned int i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    char byte = (char) strtol(byteString.c_str(), NULL, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

}  // namespace em1023
}  // namespace esphome
