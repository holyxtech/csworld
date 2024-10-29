#ifndef MESSAGE_BUILDER_H
#define MESSAGE_BUILDER_H

#include <vector>
#include <nlohmann/json.hpp>
#include "item.h"

namespace cefmsg {
  void ActionBarInit(const std::vector<std::string>& item_names);
  void ActionBarSlotChange(std::size_t index, const std::string& item_name);
  void ActionBarActiveIndex(std::size_t index);
  void ItemSelectorInit(const std::vector<std::string>& item_names);
  void ItemSelectorShow(bool show);
  void ViewChange(const std::string& view);
} // namespace cefmsg

#endif // MESSAGE_BUILDER_H