#include "material.h"

const std::string& Material::get_name() const { return name_; }
void Material::set_name(const std::string& name) { name_ = name; }
void Material::add_texture(const std::string& name) { textures_.push_back(name); }
const std::vector<std::string>& Material::get_textures() const { return textures_; }
void Material::set_blend_mode(BlendMode blend_mode) { blend_mode_ = blend_mode; }
Material::BlendMode Material::get_blend_mode() const { return blend_mode_; }
void Material::set_material_domain(MaterialDomain material_domain) { material_domain_ = material_domain; }
Material::MaterialDomain Material::get_material_domain() const { return material_domain_; }