#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <SDKDDKVer.h>
#endif
#include <asio.hpp>
#include <boost/bind/bind.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "input.h"
#include "options.h"
#include "readerwriterqueue.h"
#include "renderer.h"
#include "sim.h"
#include "tcp_client.h"

void error_callback(int errnum, const char* errmsg) {
  std::cerr << errnum << ": " << errmsg << std::endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  Input::instance()->mouse_button_callback(button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
  Input::instance()->key_callback(window, key, action);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  Input::instance()->cursor_position_callback(window, xpos, ypos);
}

int main(int argc, char* argv[]) {
  if (!Options::instance(argc, argv)->hasValidShaderPath()) {
    std::cerr << "Couldn't find App Path for Shaders, Images, etc..." << std::endl;
    return -1;
  }

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  auto window = glfwCreateWindow(Renderer::window_width, Renderer::window_height, "World", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  Input::instance()->set_prev_cursor_pos(Renderer::window_width / 2, Renderer::window_height / 2);
  Input::instance()->set_cursor_pos(Renderer::window_width / 2, Renderer::window_height / 2);

  asio::io_context io_context;

  TCPClient tcp_client(io_context);

  asio::thread t(boost::bind(&asio::io_context::run, &io_context));

  Sim sim(window, tcp_client);

  std::thread build_thread([&sim, window]() {
    auto start = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      if (duration.count() >= 16) {
        sim.step();
        start = std::chrono::high_resolution_clock::now();
      }
    }
    std::cout<<"end build loop"<<std::endl;
  });

  bool mouse_locked = true;
  bool esc_released = true;

  auto start = std::chrono::high_resolution_clock::now();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
      esc_released = true;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && esc_released) {
      esc_released = false;
      mouse_locked = !mouse_locked;
      if (mouse_locked)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (duration.count() >= 16) {
      sim.draw(duration.count());
      glfwSwapBuffers(window);
      start = std::chrono::high_resolution_clock::now();
    }
  }
  sim.exit();
  build_thread.join();
  std::cout<<"build end"<<std::endl;
  glfwTerminate();
  std::cout<<"glfw terminated"<<std::endl;
  io_context.stop();
  std::cout<<"io context stoppped"<<std::endl;
  t.join();
  std::cout<<"t end"<<std::endl;
  return 0;
}
