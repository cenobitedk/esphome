#include "esphome/core/log.h"
#include "kmp_sensor.h"

namespace esphome {
namespace kmp {

static const char *const TAG = "kmp_sensor";

KmpSensor::KmpSensor(std::string controller_id, uint16_t register_id)
    : KmpSensorBase(std::move(controller_id), std::move(register_id)) {}

void KmpSensor::update() {
  // request update
  if (this->waiting_for_data_ == false) {
    this->request_update = true;
  }
};

void KmpSensor::loop() {
  if (this->waiting_for_data_ == false && this->has_data_) {
    // Handle unit of measurement
    std::string predef_unit_ = this->get_unit_of_measurement();
    if (predef_unit_ != "" && predef_unit_ != this->unit_) {
      ESP_LOGW(TAG, "Expected unit %s but got %s", predef_unit_, this->unit_);
    }
    this->set_unit_of_measurement(this->unit_);

    // Publish value
    this->publish_state(this->data_to_be_published_);

    // Clean up
    this->clearData();
  }
}

void KmpSensor::dump_config() {
  LOG_SENSOR(TAG, "KMP", this);
  ESP_LOGCONFIG(TAG, "  Register ID: %s", this->register_id);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed!");
  }
  LOG_UPDATE_INTERVAL(this);
}



}  // namespace kmp
}  // namespace esphome
