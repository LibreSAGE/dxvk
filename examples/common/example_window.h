#pragma once

// Minimal windowing helper shared by the DXVK Native D3D examples.
//
// DXVK Native does not create windows itself. Instead, the application
// creates a window with a windowing toolkit (SDL2, SDL3 or GLFW) and passes
// the toolkit's window pointer to the D3D APIs reinterpreted as an HWND -
// exactly the same trick the WSI backends in src/wsi/ use internally.
//
// The toolkit is selected at compile time through one of the EXAMPLE_WSI_*
// macros. The matching DXVK WSI backend is selected at runtime through the
// DXVK_WSI_DRIVER environment variable, which this helper sets for you.

#include <cstdlib>

#include <windows.h>

#if defined(EXAMPLE_WSI_SDL3)
  #include <SDL3/SDL.h>
#elif defined(EXAMPLE_WSI_SDL2)
  #include <SDL.h>
#elif defined(EXAMPLE_WSI_GLFW)
  #include <GLFW/glfw3.h>
#else
  #error "No windowing backend selected (define EXAMPLE_WSI_SDL3/SDL2/GLFW)"
#endif

namespace dxvk::example {

  struct Window {
    HWND     hwnd   = nullptr;
    uint32_t width  = 0;
    uint32_t height = 0;
  };

  // Creates a window and configures the matching DXVK WSI driver. The returned
  // hwnd is the toolkit window pointer and is what gets handed to D3D.
  inline Window createWindow(const char* title, uint32_t width, uint32_t height) {
    Window window = { };
    window.width  = width;
    window.height = height;

#if defined(EXAMPLE_WSI_SDL3)
    setenv("DXVK_WSI_DRIVER", "SDL3", 1);

    if (!SDL_Init(SDL_INIT_VIDEO))
      return window;

    SDL_Window* sdlWindow = SDL_CreateWindow(
      title, int(width), int(height), SDL_WINDOW_VULKAN);

    window.hwnd = reinterpret_cast<HWND>(sdlWindow);
#elif defined(EXAMPLE_WSI_SDL2)
    setenv("DXVK_WSI_DRIVER", "SDL2", 1);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return window;

    SDL_Window* sdlWindow = SDL_CreateWindow(
      title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      int(width), int(height), SDL_WINDOW_VULKAN);

    window.hwnd = reinterpret_cast<HWND>(sdlWindow);
#elif defined(EXAMPLE_WSI_GLFW)
    setenv("DXVK_WSI_DRIVER", "GLFW", 1);

    if (!glfwInit())
      return window;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* glfwWindow = glfwCreateWindow(
      int(width), int(height), title, nullptr, nullptr);

    window.hwnd = reinterpret_cast<HWND>(glfwWindow);
#endif

    return window;
  }

  // Pumps the toolkit event queue. Returns false once the user closes the
  // window, which is the signal for the example to exit its render loop.
  inline bool processEvents(const Window& window) {
#if defined(EXAMPLE_WSI_SDL3)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        return false;
    }
    return true;
#elif defined(EXAMPLE_WSI_SDL2)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        return false;
    }
    return true;
#elif defined(EXAMPLE_WSI_GLFW)
    glfwPollEvents();
    return !glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(window.hwnd));
#endif
  }

  inline void destroyWindow(const Window& window) {
    if (!window.hwnd)
      return;

#if defined(EXAMPLE_WSI_SDL3)
    SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(window.hwnd));
    SDL_Quit();
#elif defined(EXAMPLE_WSI_SDL2)
    SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(window.hwnd));
    SDL_Quit();
#elif defined(EXAMPLE_WSI_GLFW)
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window.hwnd));
    glfwTerminate();
#endif
  }

}
