#ifndef UI_H
#define UI_H

#include <any>
#include <array>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "types.h"
#include "item.h"
#define GLM_FORCE_LEFT_HANDED
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>

class Renderer;

class UI {
public:
  struct Diff {
    struct ActionBarData {
      size_t index;
    };
    struct InventoryData {
      std::string blob;
    };
    enum Kind {
      action_bar_selection,
      inventory_selection,
    };
    Kind kind;
    std::any data;

    template <typename T>
    Diff(Kind k, const T& obj) : kind(k), data(obj) {}
  };


  UI();
  void action_bar_select(size_t index);
  void clear_diffs();
  void clear_actions();
  const std::vector<Diff>& get_diffs() const;
  const std::vector<Action>& get_actions() const;

private:
  std::vector<Diff> diffs_;
  std::vector<Action> actions_;
  std::array<Item, 10> action_bar_;
  Item active_;
};

#endif