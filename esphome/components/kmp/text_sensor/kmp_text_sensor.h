#pragma once
#include "esphome/components/kmp/kmp.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace kmp {

class KmpSensor : public KmpSensorBase, text_sensor::TextSensor, public Component {
 public:
  KmpSensor(uint16_t register_id);
  void dump_config() override;
};

}  // namespace kmp
}  // namespace esphome
