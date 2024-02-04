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

#include "ansic1218_service.h"

namespace ansic1218 {
namespace service {

class Logon : public Service {
  std::vector<uint8_t> _request;
  std::vector<uint8_t> _response;

  /*
  <logon>     ::= 50H <user-id><user>
  <user-id>   ::= <word16>            {User identification code. This field is transferred
                                       to USER_ID in Procedure 18 of C12.19.}
  <user>      ::= <byte>+10           {10 bytes containing user identification}
  */

  static constexpr uint8_t LOGON = 0x50;
  static constexpr unsigned int USER_ID_SIZE = 0x02;
  static constexpr unsigned int USER_SIZE = 0x0a;

 public:
  Logon(const std::vector<uint8_t> &identity, const std::vector<uint8_t> &user);

  bool request(std::vector<uint8_t> &buffer) override;

  bool response(std::vector<uint8_t>::const_iterator first, std::vector<uint8_t>::const_iterator last) override;
};
}  // namespace service
}  // namespace ansic1218
