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

Renderer::Renderer(World& world) : world_(world) {
  auto activate_shader = [](GLuint* shader, std::string vertex_shader_filename, std::string fragment_shader_filename) {
    auto vertex_shader_source = read_shader_file(vertex_shader_filename);
    auto fragment_shader_source = read_shader_file(fragment_shader_filename);
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
    *shader = glCreateProgram();
    glAttachShader(*shader, vertex_shader);
    glAttachShader(*shader, fragment_shader);
    glLinkProgram(*shader);
    glGetProgramiv(*shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(*shader, 512, nullptr, info);
      std::cerr << "Failed to link shader program: " << info << std::endl;
      throw std::runtime_error("Failed to initialize Renderer");
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  };

  activate_shader(&shader_, "vertex.glsl", "fragment.glsl");
  activate_shader(&window_shader_, "window_vertex.glsl", "window_fragment.glsl");

  glGenBuffers(vbos_.size(), vbos_.data());
  glGenVertexArrays(vaos_.size(), vaos_.data());

  glGenBuffers(water_vbos_.size(), water_vbos_.data());
  glGenVertexArrays(water_vaos_.size(), water_vaos_.data());

  for (int i = 0; i < vbos_.size() && i < vaos_.size(); ++i) {
    auto vbo = vbos_[i];
    auto vao = vaos_[i];
    activate_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_vertices, nullptr, GL_STATIC_DRAW);
  }

  for (int i = 0; i < water_vbos_.size() && i < water_vaos_.size(); ++i) {
    auto vbo = water_vbos_[i];
    auto vao = water_vaos_[i];
    activate_vao(vbo, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MeshGenerator::default_max_water_vertices, nullptr, GL_STATIC_DRAW);
  }

  float vertices[] = {
    -1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f};

  glGenBuffers(1, &window_vbo_);
  glGenVertexArrays(1, &window_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, window_vbo_);
  glBindVertexArray(window_vao_);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint texture_array;
  glGenTextures(1, &texture_array);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
  GLint num_layers = Voxel::num_voxel_textures;
  GLsizei width = 16;
  GLsizei height = 16;
  GLsizei num_mipmaps = 1;
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, num_mipmaps, GL_RGBA8, width, height, num_layers);

  stbi_set_flip_vertically_on_load(1);
  int _width, _height, channels;

  auto* image_data0 = stbi_load("dirt.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::tex_dirt, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data0);
  stbi_image_free(image_data0);

  auto* image_data1 = stbi_load("grass.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::tex_grass, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data1);
  stbi_image_free(image_data1);

  auto* image_data2 = stbi_load("grass_side.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::tex_grass_side, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data2);
  stbi_image_free(image_data2);

  auto* image_data3 = stbi_load("water.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::tex_water, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data3);
  stbi_image_free(image_data3);

  auto* image_data4 = stbi_load("sand.png", &_width, &_height, &channels, STBI_rgb_alpha);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, Voxel::tex_sand, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image_data4);
  stbi_image_free(image_data4);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

  GLint texture_loc = glGetUniformLocation(shader_, "textureArray");
  glUseProgram(shader_);
  glUniform1i(texture_loc, GL_TEXTURE0);

  auto activate_framebuffers = [](GLuint* fbo, GLuint* cbo, GLuint* dbo) {
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glGenTextures(1, cbo);
    glGenTextures(1, dbo);

    glBindTexture(GL_TEXTURE_2D, *cbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, *dbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *cbo, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *dbo, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  };
  activate_framebuffers(&main_framebuffer_, &main_cbo_, &main_dbo_);
  activate_framebuffers(&water_framebuffer_, &water_cbo_, &water_dbo_);

  // upload lights
  auto& sun_dir = world_.get_sun_dir();
  GLint sun_dir_loc = glGetUniformLocation(shader_, "sunDir");
  glUniform3fv(sun_dir_loc, 1, glm::value_ptr(sun_dir));
  auto& sun_col = world_.get_sun_col();
  GLint sun_col_loc = glGetUniformLocation(shader_, "sunCol");
  glUniform3fv(sun_col_loc, 1, glm::value_ptr(sun_col));
  auto& ambient_col = world_.get_ambient_col();
  GLint ambient_col_loc = glGetUniformLocation(shader_, "ambientCol");
  glUniform3fv(ambient_col_loc, 1, glm::value_ptr(ambient_col));

  glEnable(GL_DEPTH_TEST);
  /*   glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK); 
   
  glFrontFace(GL_CW);

  glClearColor(0.502f, 0.866f, 1.f, 1.0f);
}

void Renderer::activate_vao(GLuint vbo, GLuint vao) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindVertexArray(vao);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvs));
  glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, layer));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
}

void Renderer::consume_mesh_generator(MeshGenerator& mesh_generator) {
  auto& diffs = mesh_generator.get_diffs();
  for (auto& diff : diffs) {
    if (diff.kind == MeshGenerator::Diff::creation || diff.kind == MeshGenerator::Diff::water) {
      auto& loc = diff.location;

      const std::vector<Vertex>* mesh;
      std::unordered_map<GLuint, Location>* vbo_map;
      std::array<GLuint, Region::max_sz>* vbos;
      std::array<GLuint, Region::max_sz>* vaos;
      if (diff.kind == MeshGenerator::Diff::creation) {
        mesh = &mesh_generator.get_meshes().at(loc);
        vbo_map = &vbo_map_;
        vbos = &vbos_;
        vaos = &vaos_;
      } else {
        mesh = &mesh_generator.get_water_meshes().at(loc);
        vbo_map = &water_vbo_map_;
        vbos = &water_vbos_;
        vaos = &water_vaos_;
      }

      GLuint vbo_to_use = 0;
      GLuint vao = 0;
      GLuint* vbo_ptr = nullptr;
      for (int i = 0; i < vbos_.size(); ++i) {
        auto& vbo = (*vbos)[i];
        if (!vbo_map->contains(vbo)) {
          vbo_to_use = vbo;
          vbo_ptr = &(*vbos)[i];
          vao = (*vaos)[i];
          break;
        }
      }

      glBindBuffer(GL_ARRAY_BUFFER, vbo_to_use);
      GLint bufferSize;
      glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
      if (bufferSize < sizeof(Vertex) * mesh->size()) {
        std::cout << "Buffer too small! This mesh has size " << mesh->size() << std::endl;

        glDeleteBuffers(1, vbo_ptr);
        glGenBuffers(1, vbo_ptr);
        vbo_to_use = *vbo_ptr;
        activate_vao(vbo_to_use, vao);

        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->size(), mesh->data(), GL_STATIC_DRAW);
      } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * mesh->size(), mesh->data());
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      if (diff.kind == MeshGenerator::Diff::creation) {
        (*vbo_map)[vbo_to_use] = loc;
        loc_map_[loc] = vbo_to_use;
        mesh_size_map_[vbo_to_use] = mesh->size();
      } else {
        (*vbo_map)[vbo_to_use] = loc;
        water_loc_map_[loc] = vbo_to_use;
        water_mesh_size_map_[vbo_to_use] = mesh->size();
      }

    } else if (diff.kind == MeshGenerator::Diff::deletion) {
      GLuint vbo = loc_map_[diff.location];
      vbo_map_.erase(vbo);
      GLuint water_vbo = water_loc_map_[diff.location];
      water_vbo_map_.erase(water_vbo);
    }
  }

  mesh_generator.clear_diffs();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::consume_camera(const Camera& camera) {

  glm::mat4 projection = glm::perspective(glm::radians(55.l), 16 / 9.l, 0.1l, 4000.l);
  glm::mat4 view = camera.get_view();
  glm::mat4 transform = projection * view;

  //  glm::mat4 transform = projection * view;

  auto& camera_pos = camera.get_position();

  auto transform_loc = glGetUniformLocation(shader_, "uTransform");
  auto camera_pos_loc = glGetUniformLocation(shader_, "uCameraPos");
  glUseProgram(shader_);
  glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &transform[0][0]);
  glUniform3fv(camera_pos_loc, 1, glm::value_ptr(camera_pos));
}

void Renderer::render() const {
  glUseProgram(shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, main_framebuffer_);
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

  glBindFramebuffer(GL_FRAMEBUFFER, water_framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (int i = 0; i < water_vbos_.size() && i < water_vaos_.size(); ++i) {
    auto vbo = water_vbos_[i];
    auto vao = water_vaos_[i];

    if (!(water_vbo_map_.contains(vbo) && water_mesh_size_map_.at(vbo) > 0))
      continue;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);

    int mesh_size = water_mesh_size_map_.at(vbo);
    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
  }

  glUseProgram(window_shader_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, main_cbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "MainColor"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, water_cbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "WaterColor"), 1);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, main_dbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "MainDepth"), 2);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, water_dbo_);
  glUniform1i(glGetUniformLocation(window_shader_, "WaterDepth"), 3);
  glBindBuffer(GL_ARRAY_BUFFER, window_vbo_);
  glBindVertexArray(window_vao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
