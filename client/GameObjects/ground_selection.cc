#include "ground_selection.h"
#include <glm/ext.hpp>
#include "../input.h"
#include "../renderer.h"
#include "../sim.h"

GroundSelection::GroundSelection() {
  set_scene_component(std::make_unique<SceneComponent>());
  auto& vertices = scene_component_->get_vertices();
  vertices.resize(100000);
  scene_component_->set_flag(SceneComponentFlags::Dynamic);
  scene_component_->set_primitive_type(PrimitiveType::Triangles);
  auto& vertex_attributes = scene_component_->get_vertex_attributes();
  VertexAttribute vertex_attribute;
  vertex_attribute.type = VertexAttributeType::Float;
  vertex_attribute.component_count = 3;
  vertex_attributes.push_back(vertex_attribute);
  Material material;
  material.set_name("GroundSelection");
  material.set_blend_mode(Material::BlendMode::Opaque);
  material.set_material_domain(Material::MaterialDomain::PostProcess);
  material.add_texture("MainDepth");
  material.add_texture("MainColor");
  scene_component_->set_material(material);
  scene_component_->set_model_transform(glm::mat4(1));
}

void GroundSelection::process_input(const InputEvent& event) {
  switch (event.kind) {
  case InputEvent::MouseButtonEvent: {
    auto& mouse_button_event = std::any_cast<const MouseButtonEvent&>(event.data);
    if (mouse_button_event.action == GLFW_PRESS && mouse_button_event.button == GLFW_MOUSE_BUTTON_LEFT)
      doing_selection_ = true;
    else if (mouse_button_event.action == GLFW_RELEASE && mouse_button_event.button == GLFW_MOUSE_BUTTON_LEFT)
      doing_selection_ = false;
  } break;
  }
}
void GroundSelection::step() {
  if (!doing_selection_)
    return;

  auto& sim = get_sim();
  auto& renderer = sim.get_renderer();
  auto& camera = sim.get_camera();
  auto& region = sim.get_region();
  auto& ui = sim.get_ui();
  auto& brush = ui.get_brush();
  auto& mesh_generator = sim.get_mesh_generator();

  auto [mouse_x, mouse_y] = Input::instance()->get_cursor_pos();
  float ndc_x = (2.0f * mouse_x) / Renderer::window_width - 1.0f;
  float ndc_y = 1.0f - (2.0f * mouse_y) / Renderer::window_height;
  glm::dvec4 ndc_space_pos(ndc_x, ndc_y, -1.0f, 1.0f);
  glm::dmat4 inverted_projection = glm::inverse(renderer.get_projection_matrix());
  // this is the position on the near plane with these mouse coordinates
  auto view_space_pos = inverted_projection * ndc_space_pos;
  if (view_space_pos.w != 0)
    view_space_pos /= view_space_pos.w;
  auto inverted_view = glm::inverse(camera.get_raw_view());
  glm::dvec3 world_space_pos = inverted_view * view_space_pos;
  glm::dvec3 direction = glm::normalize(world_space_pos - camera.get_position());

  auto is_dirt = [](Voxel v) -> bool { return v == Voxel::dirt; };
  auto is_not_empty = [](Voxel v) -> bool { return v != Voxel::empty; };

  Int3D coord;
  Voxel voxel_at_cursor;
  bool success = region.get_first_of_kind_without_obstruction(
    world_space_pos, direction, 500, coord, voxel_at_cursor,
    is_dirt,
    is_not_empty);

  // check y+1. if something, do nothing
  if (!success)
    return;
  /*   auto above_coord = Int3D{coord[0], coord[1] + 1, coord[2]};
    auto loc = Region::location_from_global_coord(above_coord);
    if (!region.has_chunk(loc))
      return;
    Voxel above_voxel = region.get_voxel(above_coord);
    if (!vops::is_empty(above_voxel))
      return; */

  // int brush_radius = brush.brush_radius;
  int brush_radius = 2;
  std::vector<Voxel> voxels;
  voxels.reserve(4);
  for (int z = -brush_radius; z <= brush_radius; ++z) {
    for (int x = -brush_radius; x <= brush_radius; ++x) {
      glm::dvec3 adjacent_pos_center{coord[0] + x + 0.5, coord[1] - .5, coord[2] + z + 0.5};

      voxels.clear();
      bool found = region.get_until_kind(adjacent_pos_center, glm::dvec3(0, 1, 0), 4, voxels, vops::is_empty);
      if (!found)
        continue;
      int offset = 2;
      if (voxels.size() == 0) {
        // handle edge case
        adjacent_pos_center[1] += 1;
        voxels.clear();
        found = region.get_until_kind(adjacent_pos_center, glm::dvec3(0, 1, 0), 3, voxels, vops::is_empty);
        if (!found)
          continue;
        offset = 1;
      }

      if (voxels.size() == 0 || voxels.back() != Voxel::dirt)
        continue;

      Int3D adjacent_coord{coord[0] + x, coord[1] - offset + static_cast<int>(voxels.size()), coord[2] + z};
      selected_.insert(adjacent_coord);
    }
  }

  auto& origin = mesh_generator.get_origin();
  glm::dvec3 world_offset = glm::dvec3(origin[0] * Chunk::sz_x, origin[1] * Chunk::sz_y, origin[2] * Chunk::sz_z);

  // 2. mesh/update scene component
  std::vector<glm::vec3> vertices;
  double y_offset = 1.0;
  vertices.reserve(selected_.size() * 6);
  for (auto& coord : selected_) {
    glm::vec3 v = glm::dvec3{coord[0], coord[1] + y_offset, coord[2]} - world_offset;
    vertices.push_back(v);
    v[2] += 1;
    vertices.push_back(v);
    v[0] += 1;
    vertices.push_back(v);
    v[0] -= 1;
    v[2] -= 1;
    vertices.push_back(v);
    v[0] += 1;
    v[2] += 1;
    vertices.push_back(v);
    v[2] -= 1;
    vertices.push_back(v);
  }
  std::size_t vertex_byte_count = sizeof(glm::vec3) * vertices.size();
  scene_component_->memcpy_vertices(vertices.data(), vertex_byte_count);
  scene_component_->set_vertex_count(vertices.size());
}