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

#include <sstream>
#include "table.h"

using namespace std;
using namespace esphome::ansic1218::table;

Table::Table(uint16_t id) : _id(id), _offset(0) {}

Table::Table(uint16_t id, uint32_t offset) : _id(id), _offset(offset) {}

Table::Table(uint16_t id, uint32_t offset, uint16_t count) : _id(id), _offset(offset), _count(count) {
  raw_data.reserve(_count);
  raw_data.clear();
}

uint16_t Table::id() const { return _id; }

uint32_t Table::offset() const { return _offset; }

uint32_t Table::count() const { return _count; }

vector<uint8_t> &Table::data() { return raw_data; }

string Table::name() const {
  ostringstream out;
  out << "Table" << _id;
  return out.str();
}

bool Table::response(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  if (!validate(first, last))
    return false;

  raw_data.reserve(distance(first, last));
  raw_data.clear();

  copy(first, last, back_inserter(raw_data));

  return true;
}

bool Table::validate(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  return 0 < distance(first, last);
}

void Table::initializer(vector<uint8_t>::const_iterator first, vector<uint8_t>::const_iterator last) {
  raw_data.reserve(distance(first, last));
  raw_data.clear();

  copy(first, last, back_inserter(raw_data));
}
