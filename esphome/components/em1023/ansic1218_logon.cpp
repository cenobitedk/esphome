#ifdef USE_ESP32

#include "ansic1218_logon.h"

using namespace std;
using namespace esphome;
using namespace ansic1218::service;

static const char *TAG = "ansic1218::services:logon";

Logon::Logon(const std::vector<uint8_t> &identity, const std::vector<uint8_t> &user) : Service(__PRETTY_FUNCTION__) {
  if (identity.size() < USER_ID_SIZE) {
    ESP_LOGE(TAG, "Invalid identity size");
    return;
  }

  if (user.size() < USER_SIZE) {
    ESP_LOGE(TAG, "Invalid user size");
    return;
  }

  _request.resize(USER_ID_SIZE + USER_SIZE);
  _request.clear();
  _request.push_back(uint8_t(LOGON));

  copy(identity.begin(), identity.begin() + std::min(identity.size(), static_cast<unsigned int>(USER_ID_SIZE)),
       back_inserter(_request));

  copy(user.begin(), user.begin() + std::min(user.size(), static_cast<unsigned int>(USER_SIZE)),
       back_inserter(_request));
}

bool Logon::request(std::vector<uint8_t> &buffer) {
  copy(_request.cbegin(), _request.cend(), back_inserter(buffer));

  return true;
}

bool Logon::response(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  return Service::validate(first, last);
}

#endif
