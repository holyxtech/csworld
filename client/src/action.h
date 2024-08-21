#include <any>
#include <optional>
#include "item.h"

struct Action {
  struct TerrainGenerationData {
  };
  enum Kind {
    terrain_generation
  };
  Kind kind;
  std::any data;

  Action() = default;
  template <typename T>
  Action(Kind k, const T& obj) : kind(k), data(obj) {}
};
