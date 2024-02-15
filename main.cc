#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <asio.hpp>
#include <boost/bind/bind.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "input.h"
#include "readerwriterqueue.h"
#include "sim.h"
#include "sim_server.h"
#include "tcp_client.h"

void error_callback(int errnum, const char* errmsg) {
  std::cerr << errnum << ": " << errmsg << std::endl;
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
  Input::instance()->key_callback(window, key, action);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  Input::instance()->cursor_position_callback(window, xpos, ypos);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  const GLuint width = 1920, height = 1080;
  auto window = glfwCreateWindow(width, height, "World", nullptr, nullptr);
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

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);

  asio::io_context io_context;
  TCPClient tcp_client(io_context);
  asio::thread t(boost::bind(&asio::io_context::run, &io_context));

  Sim sim(window, tcp_client);

  std::thread build_thread([&sim, window]() {
    while (!glfwWindowShouldClose(window)) {
      sim.step();
    }
  });

  auto start = std::chrono::high_resolution_clock::now();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    sim.draw(duration.count());
    start = std::chrono::high_resolution_clock::now();
    glfwSwapBuffers(window);
    
  }
  build_thread.join();

  glfwTerminate();
  io_context.stop();
  t.join();

  return 0;
}
