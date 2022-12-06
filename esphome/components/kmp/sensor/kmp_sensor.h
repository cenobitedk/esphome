#pragma once
#include "esphome/components/kmp/kmp.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace kmp {

class KmpSensor : public KmpSensorBase, public sensor::Sensor, public PollingComponent {
 public:
  KmpSensor(std::string controller_id, uint16_t register_id);
  void update() override;
  void dump_config() override;
};

}  // namespace kmp
}  // namespace esphome
