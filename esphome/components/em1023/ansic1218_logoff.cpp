#ifdef USE_ESP32

#include "ansic1218_logoff.h"

using namespace std;
using namespace esphome;
using namespace ansic1218::service;

static const char *TAG = "ansic1218::services:logoff";

Logoff::Logoff() : Service(__PRETTY_FUNCTION__) {}

bool Logoff::request(std::vector<uint8_t> &buffer) {
  buffer.push_back(LOGOFF);

  return true;
}

bool Logoff::response(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  return Service::validate(first, last);
}

#endif
