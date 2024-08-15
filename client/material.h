#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <vector>

class Material {
public:
  enum BlendMode {
    Opaque,
    Translucent
  };
  enum MaterialDomain {
    Surface,
    PostProcess,
  };
  Material() = default;
  const std::string& get_name() const;
  void set_name(const std::string& name);
  void add_texture(const std::string& name);
  const std::vector<std::string>& get_textures() const;
  void set_blend_mode(BlendMode blend_mode);
  BlendMode get_blend_mode() const;
  void set_material_domain(MaterialDomain material_domain);
  MaterialDomain get_material_domain() const;

private:
  std::string name_; // for now directly refers to shader
  std::vector<std::string> textures_;
  BlendMode blend_mode_;
  MaterialDomain material_domain_;
};

#endif