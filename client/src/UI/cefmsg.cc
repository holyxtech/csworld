#include "cefmsg.h"
#include <string>
#include "cefui.h"

namespace {
  void SendMessage(const std::string& type, const nlohmann::json& payload) {
    cefui::SendMessageToJS(
      {{"type", type},
       {"payload", payload}});
  }

} // namespace

namespace cefmsg {
  void ActionBarSlotChange(std::size_t index, std::string& item_name) {
    SendMessage(
      "actionBarSlotChange",
      {{"index", index},
       {"item", item_name}});
  }
} // namespace cefmsg