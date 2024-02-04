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
#include "ansic1218_security.h"
#include "mbedtls/md.h"

// // Copied from https://github.com/Mbed-TLS/mbedtls/blob/development/include/mbedtls/md.h
// #define MBEDTLS_MD_MAX_SIZE 32

using namespace std;
using namespace esphome;
using namespace ansic1218::service;

static const char *TAG = "ansic1218::services:security";

// Security::Security(const vector<uint8_t> &identity, const vector<uint8_t> &password)
Security::Security(const vector<uint8_t> &key) : Service(__PRETTY_FUNCTION__), _request(), _response() {
  // if (identity.size() < IDENTITY_LAST_N_BYTES) {
  //   ESP_LOGE(TAG, "Invalid identity size");
  //   return;
  // }

  // ESP_LOGD(TAG, "Decryption key: %s", format_hex_pretty(key).c_str());

  // if (password.size() < SECURITY_PASSWORD_SIZE) {
  //   ESP_LOGE(TAG, "Invalid password size: %u, expected %u", password.size(), SECURITY_PASSWORD_SIZE);
  //   return;
  // }

  // unsigned int md_len = MBEDTLS_MD_MAX_SIZE;
  // vector<uint8_t> md(md_len);

  // // Skip - cannot load mbedtls lib.
  // if (mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), password.data(), password.size(),
  //                     identity.data() + identity.size() - IDENTITY_LAST_N_BYTES, IDENTITY_LAST_N_BYTES, md.data())) {
  //   ESP_LOGE(TAG, "Couldn't generate HMAC");
  // }

  if (key.size() < SECURITY_KEY_SIZE) {
    ESP_LOGE(TAG, "Invalid key size: %u, expected %u", key.size(), SECURITY_KEY_SIZE);
    return;
  }

  // _request.resize(md_len);
  _request.clear();
  _request.push_back(uint8_t(SECURITY));

  // copy(md.begin(), md.begin() + std::min(md_len, static_cast<unsigned int>(SECURITY_PASSWORD_SIZE)),
  //      back_inserter(_request));

  copy(key.begin(), key.begin() + std::min(key.size(), static_cast<unsigned int>(SECURITY_KEY_SIZE)),
       back_inserter(_request));
}

bool Security::request(std::vector<uint8_t> &buffer) {
  copy(_request.cbegin(), _request.cend(), back_inserter(buffer));

  return true;
}

bool Security::response(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  return Service::validate(first, last);
}

#endif
