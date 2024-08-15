#include "scene_component.h"

SceneComponent::SceneComponent() {}
std::uint32_t SceneComponent::get_id() const { return id_; }
void SceneComponent::set_id(std::uint32_t id) { id_ = id; }
const Material& SceneComponent::get_material() const { return material_; }
void SceneComponent::set_material(const Material& mat) { material_ = mat; }
const glm::mat4& SceneComponent::get_model_transform() const { return model_transform_; }
void SceneComponent::set_model_transform(const glm::mat4& transform) { model_transform_ = transform; }
const std::vector<std::uint8_t>& SceneComponent::get_vertices() const { return vertices_; }
const std::vector<unsigned int>& SceneComponent::get_indices() const { return indices_; }
void SceneComponent::set_vertices(std::vector<std::uint8_t>&& vertices) { vertices_ = std::move(vertices); }
void SceneComponent::set_indices(std::vector<unsigned int>&& indices) { indices_ = std::move(indices); }
const std::vector<VertexAttribute>& SceneComponent::get_vertex_attributes() const { return vertex_attributes_; }
void SceneComponent::set_vertex_attributes(std::vector<VertexAttribute>&& attributes) { vertex_attributes_ = std::move(attributes); }
void SceneComponent::set_vertex_count(unsigned int count) { vertex_count_ = count; }
unsigned int SceneComponent::get_vertex_count() const { return vertex_count_; }
PrimitiveType SceneComponent::get_primitive_type() const { return primitive_type_; }
void SceneComponent::set_primitive_type(PrimitiveType type) { primitive_type_ = type; }