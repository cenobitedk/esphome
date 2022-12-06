#include "esphome/core/log.h"
#include "kmp_sensor.h"

namespace esphome {
namespace kmp {

static const char *const TAG = "kmp_sensor";

KmpSensor::KmpSensor(std::string controller_id, uint16_t register_id)
    : KmpSensorBase(std::move(controller_id), std::move(register_id)) {}

void KmpSensor::update(){
    // queue command
};

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
