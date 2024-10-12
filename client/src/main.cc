#ifdef _WIN32
#include <windows.h>

#include <SDKDDKVer.h>
#include <shellapi.h>
#include "UI/cefui_win.h"
#endif
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include <nuklear.h>
#undef NK_IMPLEMENTATION

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

// Helper function to convert wide-char arguments to char* argv[]
char** convert_wide_args(int argc, LPWSTR* wargv) {
  // Allocate memory for argv
  char** argv = new char*[argc];

  for (int i = 0; i < argc; ++i) {
    // Get the size needed to store the converted narrow string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);

    // Allocate memory for each argument (char*)
    argv[i] = new char[size_needed];

    // Convert wide char to narrow char (UTF-8)
    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size_needed, NULL, NULL);
  }

  return argv;
}

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  int argc;
  LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
  char** argv = convert_wide_args(argc, wargv);
  LocalFree(wargv);
#else
int main(int argc, char* argv[]) {
#endif

  try {
    Options::instance(argc, argv);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return -1;
  }
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

#ifdef _WIN32
  /*   std::thread cef_thread([&hInstance]() {

    }); */
  cefui::main(hInstance);
#endif

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(Renderer::window_width, Renderer::window_height, "World", nullptr, nullptr);
  if (window == nullptr) {
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
  double xpos = Renderer::window_width / 2;
  double ypos = Renderer::window_height / 2;
  Input::instance()->set_cursor_pos(xpos, ypos);
  glfwSetCursorPos(window, xpos, ypos);

  asio::io_context io_context;
  TCPClient tcp_client(io_context);
  asio::thread t(boost::bind(&asio::io_context::run, &io_context));

  Sim sim(window, tcp_client);
  bool quit = false;
  std::thread game_thread([&sim, &quit]() {
    auto start = std::chrono::high_resolution_clock::now();
    while (!quit) {
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      if (duration.count() >= 16) {
        sim.step(duration.count());
        start = std::chrono::high_resolution_clock::now();
      }
    }
  });

  auto start = std::chrono::high_resolution_clock::now();
  while (!quit) {
    glfwPollEvents();
    cefui::domessage();
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      quit = true;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (duration.count() >= 16) {
      sim.draw(duration.count());
      glfwSwapBuffers(window);
      start = std::chrono::high_resolution_clock::now();
    }
  }
  sim.exit();
  game_thread.join();
  sim.save();
  glfwTerminate();
  io_context.stop();
  t.join();
  cefui::shutdown();
  return 0;
}
