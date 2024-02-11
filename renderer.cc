#include "renderer.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtc/random.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "types.h"

static std::string read_shader_file(std::string path) {
  std::ifstream fs(path, std::ios::in);

  if (!fs.is_open()) {
    std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
    return "";
  }
  return std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
}

Renderer::Renderer() {
  auto vertex_shader_source = read_shader_file("vertex.glsl");
  auto fragment_shader_source = read_shader_file("fragment.glsl");

  const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  auto* vertex_shader_source_cstr = vertex_shader_source.c_str();
  glShaderSource(vertex_shader, 1, &vertex_shader_source_cstr, nullptr);
  glCompileShader(vertex_shader);
  GLint success;
  GLchar info[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, nullptr, info);
    std::cerr << "Failed to compile vertex shader: " << info << std::endl;
    throw std::runtime_error("Failed to initialize Renderer");
  }

  const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  auto* fragment_shader_source_cstr = fragment_shader_source.c_str();
  glShaderSource(fragment_shader, 1, &fragment_shader_source_cstr, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
    std::cerr << "Failed to compile fragment shader: " << info << std::endl;
    throw std::runtime_error("Failed to initialize Renderer");
  }

  shader_ = glCreateProgram();
  glAttachShader(shader_, vertex_shader);
  glAttachShader(shader_, fragment_shader);
  glLinkProgram(shader_);
  glGetProgramiv(shader_, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_, 512, nullptr, info);
    std::cerr << "Failed to link shader program: " << info << std::endl;
    throw std::runtime_error("Failed to initialize Renderer");
  }
  glUseProgram(shader_);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glGenBuffers(vbos_.size(), vbos_.data());
  glGenVertexArrays(vaos_.size(), vaos_.data());

  for (int i = 0; i < vbos_.size() && i < vaos_.size(); ++i) {
    auto vbo = vbos_[i];
    auto vao = vaos_[i];

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
    glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, layer));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * chunk_max_vertices_, nullptr, GL_STATIC_DRAW);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint texture_array;
  glGenTextures(1, &texture_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
  GLint num_layers = 4;
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_RGBA8, width, height, num_layers);

  stbi_set_flip_vertically_on_load(1);
  int _width, _height, channels;

  auto* image_data0 = stbi_load("dirt.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::textures.at(Voxel::tex_dirt), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data0);
  stbi_image_free(image_data0);

  auto* image_data1 = stbi_load("grass.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::textures.at(Voxel::tex_grass), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data1);
  stbi_image_free(image_data1);

  auto* image_data2 = stbi_load("grass_side.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::textures.at(Voxel::tex_grass_side), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data2);
  stbi_image_free(image_data2);

  auto* image_data3 = stbi_load("water.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::textures.at(Voxel::tex_water), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data3);
  stbi_image_free(image_data3);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

  GLint texture_loc = glGetUniformLocation(shader_, "textureArray");
  glUniform1i(texture_loc, GL_TEXTURE0);

  glEnable(GL_DEPTH_TEST);
  // glFrontFace(GL_CCW);
  // glEnable(GL_CULL_FACE);
  glClearColor(0.612f, 0.914f, 1.f, 1.0f);
}

void Renderer::consume_mesh_generator(MeshGenerator& mesh_generator) {
  auto& diffs = mesh_generator.get_diffs();
  auto& meshes = mesh_generator.get_meshes();

  for (auto& diff : diffs) {
    if (diff.kind == MeshGenerator::Diff::creation) {
      auto& loc = diff.location;
      auto& mesh = meshes.at(loc);

      GLuint vbo_to_use = 0;
      for (auto vbo : vbos_) {
        if (!vbo_map_.contains(vbo)) {
          vbo_to_use = vbo;
          break;
        }
      }

      glBindBuffer(GL_ARRAY_BUFFER, vbo_to_use);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * mesh.size(), mesh.data());
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      vbo_map_[vbo_to_use] = loc;
      loc_map_[loc] = vbo_to_use;
      mesh_size_map_[vbo_to_use] = mesh.size();

    } else if (diff.kind == MeshGenerator::Diff::deletion) {
      GLuint vbo = loc_map_[diff.location];
      vbo_map_.erase(vbo);
    }
  }

  mesh_generator.clear_diffs();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::consume_camera(const Camera& camera) {
  glm::mat4 projection = glm::perspective(glm::radians(45.l), 16 / 9.l, 0.1l, 400.l);
  glm::mat4 view = camera.get_view();
  glm::mat4 transform = projection * view;

  auto transform_loc = glGetUniformLocation(shader_, "transform");

  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);
}

void Renderer::render() const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (int i = 0; i < vbos_.size() && i < vaos_.size(); ++i) {
    auto vbo = vbos_[i];
    auto vao = vaos_[i];

    if (!vbo_map_.contains(vbo))
      continue;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);

    int mesh_size = mesh_size_map_.at(vbo);
    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
