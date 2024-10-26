#ifndef MESSAGE_BUILDER_H
#define MESSAGE_BUILDER_H

#include <nlohmann/json.hpp>
#include "item.h"

namespace cefmsg {
  void ActionBarSlotChange(std::size_t index, std::string& item_name);
}

#endif // MESSAGE_BUILDER_H