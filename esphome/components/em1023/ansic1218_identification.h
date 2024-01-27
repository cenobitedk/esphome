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

#pragma once

// #include "esp_log.h"
#include "esphome/core/log.h"
#include "ansic1218_service.h"

namespace ansic1218 {
namespace service {

class Identification : public Service {
  std::vector<uint8_t> content;

 public:
  struct Response;
  struct DeviceIdentity;

  Identification();
  virtual ~Identification() = default;

  bool request(std::vector<uint8_t> &buffer) override;

  bool response(std::vector<uint8_t>::const_iterator first, std::vector<uint8_t>::const_iterator last) override;

  std::vector<uint8_t> getDeviceIdentity();
};

}  // namespace service
}  // namespace ansic1218
