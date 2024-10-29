#include "cefmsg.h"
#include <string>
#include "cefui.h"

namespace {
  void SendMessageToJS(const std::string& type, const nlohmann::json& payload) {
    cefui::SendMessageToJS(
      {{"type", type},
       {"payload", payload}});
  }

} // namespace

namespace cefmsg {
  void ActionBarInit(const std::vector<std::string>& item_names) {
    SendMessageToJS("actionBarInit", {{"itemNames", item_names}});
  }
  void ActionBarSlotChange(std::size_t index, const std::string& item_name) {
    SendMessageToJS(
      "actionBarSlotChange",
      {{"index", index},
       {"item", item_name}});
  }
  void ActionBarActiveIndex(std::size_t index) {
    SendMessageToJS(
      "actionBarActiveIndex",
      {{"index", index}});
  }
  void ItemSelectorInit(const std::vector<std::string>& item_names) {
    SendMessageToJS(
      "itemSelectorInit",
      {{"itemNames", item_names}});
  }
  void ItemSelectorShow(bool show) {
    SendMessageToJS(
      "itemSelectorShow",
      {{"show", show}});
  }
  void ViewChange(const std::string& view) {
    SendMessageToJS(
      "viewChange",
      {{"view", view}});
  }
} // namespace cefmsg