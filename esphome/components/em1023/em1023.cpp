#include "em1023.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include "em1023.h"

using namespace ansic1218;
using namespace ansic1218::service;
using namespace ansic1218::table;

namespace esphome {
namespace em1023 {

static const char *const TAG = "em1023";

void EM1023Component::setup() { transport_ = new Transport(this); }

void EM1023Component::update() { this->trigger_update_ = true; }

void EM1023Component::loop() {
  if (this->trigger_update_) {
    ESP_LOGI(TAG, "update() start");
    this->trigger_update_ = false;

    // vector<uint8_t> identity{};

    // Identification *identification = new Identification();

    if (!transport_->request(Identification())) {
      ESP_LOGE(TAG, "Could not request identity service");
      goto logoff;
      // return;
    } else {
      ESP_LOGI(TAG, "Identity service request completed");
    }

    // identity = identification->getDeviceIdentity();

    // delete identification;

    // if (!transport_->request(Logon({0x00, 0x00}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}))) {
    if (!transport_->request(Logon({0x11, 0x11}, {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09}))) {
      ESP_LOGE(TAG, "Could not request logon service");
      // return;
      goto logoff;
    }

    // ESP_LOGD(TAG, "Decryption key: %s", format_hex_pretty(this->decryption_key_).c_str());

    if (!transport_->request(Security(this->decryption_key_))) {
      ESP_LOGE(TAG, "could not request security service");
      // return;
      goto logoff;
    }

    // Table01 *table01 = new Table01();

    // if (!transport_->request(ReadFull(*table01))) {
    //   ESP_LOGE(TAG, "could not request Table01 service");
    //   // return;
    //   goto logoff;
    // } else {
    //   auto info_01 = table01->content();
    //   string meter_serial(info_01->mfg_serial_number, sizeof(info_01->mfg_serial_number));
    //   ostringstream ofw;
    //   ostringstream ohw;

    //   ofw << (int) info_01->fw_version_number << "." << (int) info_01->fw_revision_number;
    //   ohw << (int) info_01->hw_version_number << "." << (int) info_01->hw_revision_number;

    //   ESP_LOGI(TAG, "Meter firmware: %s", ofw.str().c_str());
    //   ESP_LOGI(TAG, "Meter hardware: %s", ohw.str().c_str());
    //   ESP_LOGI(TAG, "Meter serial: %s", meter_serial.c_str());
    // }

  logoff:
    if (!transport_->request(Logoff())) {
      ESP_LOGE(TAG, "Could not request logoff service");
    }

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
  if (decryption_key.length() != 20) {
    ESP_LOGE(TAG, "Error, decryption key must be 20 character long");
    return;
  }

  this->decryption_key_.clear();

  // Verbose level prints decryption key
  ESP_LOGD(TAG, "Using decryption key: %s", decryption_key.c_str());

  parse_hex(decryption_key, this->decryption_key_, decryption_key.length() / 2);
}

}  // namespace em1023
}  // namespace esphome
