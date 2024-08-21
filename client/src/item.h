#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <unordered_map>
#include "voxel.h"

using Item = std::uint32_t;

namespace ItemUtils {
  const std::unordered_map<std::string, Item> items = []() {
    std::unordered_map<std::string, Item> items;
    std::uint32_t item_id = 0;
    auto item_names = std::vector{
      "dirt",
      "stone",
      "sandstone",
      "water",
      "leaves",
      "tree_trunk",
      "standing_grass",
      "roses",
      "sunflower",
      "bricks"};
    for (auto name : item_names) {
      items.insert({name, item_id++});
    }
    return items;
  }();

  const std::unordered_map<Item, Voxel> item_to_voxel{
    {items.at("dirt"), Voxel::dirt},
    {items.at("stone"), Voxel::stone},
    {items.at("sandstone"), Voxel::sandstone},
    {items.at("water"), Voxel::water_full},
    {items.at("standing_grass"), Voxel::grass},
    {items.at("roses"), Voxel::roses},
    {items.at("sunflower"), Voxel::sunflower},
    {items.at("leaves"), Voxel::leaves},
    {items.at("tree_trunk"), Voxel::tree_trunk},
    {items.at("bricks"), Voxel::bricks},
  };

} // namespace ItemUtils

#endif