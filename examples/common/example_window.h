#pragma once

// Minimal windowing helper shared by the D3D examples.
//
// The examples create a window with a windowing toolkit (SDL2, SDL3 or GLFW)
// and hand the resulting HWND to the D3D APIs. How that HWND is obtained
// differs per platform:
//
//  * DXVK Native: DXVK's WSI backends define an HWND to *be* the toolkit's
//    window pointer, so we simply reinterpret it (see include/native/wsi/).
//    The backend is picked at runtime via the DXVK_WSI_DRIVER variable, which
//    this helper sets to match the toolkit it was compiled against.
//  * Windows: HWND is a real OS handle, so we ask the toolkit for the native
//    handle of the window it created. DXVK only has the Win32 WSI there, so
//    DXVK_WSI_DRIVER must be left alone.
//
// The toolkit is selected at compile time via the EXAMPLE_WSI_* macros.
//
// Set DXVK_EXAMPLE_FRAMES=<n> to render n frames and exit; this is what CI
// uses to run the examples non-interactively.

#include <cstdint>
#include <cstdlib>

#include <windows.h>

#if defined(EXAMPLE_WSI_SDL3)
  #include <SDL3/SDL.h>
#elif defined(EXAMPLE_WSI_SDL2)
  #include <SDL.h>
  #if defined(_WIN32)
    #include <SDL_syswm.h>
  #endif
#elif defined(EXAMPLE_WSI_GLFW)
  #include <GLFW/glfw3.h>
  #if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
  #endif
#else
  #error "No windowing backend selected (define EXAMPLE_WSI_SDL3/SDL2/GLFW)"
#endif

namespace dxvk::example {

  struct Window {
    // Handle handed to D3D. On Native this aliases 'handle'; on Windows it is
    // the real OS window handle owned by the toolkit.
    HWND     hwnd       = nullptr;
    // The toolkit's own window object, used for event pumping and teardown.
    void*    handle     = nullptr;
    uint32_t width      = 0;
    uint32_t height     = 0;
    uint32_t frameLimit = 0;  // 0 = run until the window is closed
    uint32_t frameCount = 0;
  };

  namespace detail {

    // On DXVK Native the WSI backend is chosen at runtime and must match the
    // toolkit we created the window with. On Windows the only backend is Win32,
    // so setting this would make WSI initialization fail.
    inline void selectWsiDriver([[maybe_unused]] const char* driver) {
#if !defined(_WIN32)
      setenv("DXVK_WSI_DRIVER", driver, 1);
#endif
    }

    inline uint32_t frameLimitFromEnv() {
      const char* env = std::getenv("DXVK_EXAMPLE_FRAMES");
      return env ? uint32_t(std::strtoul(env, nullptr, 10)) : 0;
    }

  }

  // Creates a window and returns the handle to pass to D3D.
  inline Window createWindow(const char* title, uint32_t width, uint32_t height) {
    Window window = { };
    window.width      = width;
    window.height     = height;
    window.frameLimit = detail::frameLimitFromEnv();

#if defined(EXAMPLE_WSI_SDL3)
    detail::selectWsiDriver("SDL3");

    if (!SDL_Init(SDL_INIT_VIDEO))
      return window;

    SDL_Window* sdlWindow = SDL_CreateWindow(
      title, int(width), int(height), SDL_WINDOW_VULKAN);

    if (!sdlWindow)
      return window;

    window.handle = sdlWindow;

  #if defined(_WIN32)
    window.hwnd = reinterpret_cast<HWND>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindow),
      SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
  #else
    window.hwnd = reinterpret_cast<HWND>(sdlWindow);
  #endif
#elif defined(EXAMPLE_WSI_SDL2)
    detail::selectWsiDriver("SDL2");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return window;

    SDL_Window* sdlWindow = SDL_CreateWindow(
      title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      int(width), int(height), SDL_WINDOW_VULKAN);

    if (!sdlWindow)
      return window;

    window.handle = sdlWindow;

  #if defined(_WIN32)
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (SDL_GetWindowWMInfo(sdlWindow, &wmInfo))
      window.hwnd = wmInfo.info.win.window;
  #else
    window.hwnd = reinterpret_cast<HWND>(sdlWindow);
  #endif
#elif defined(EXAMPLE_WSI_GLFW)
    detail::selectWsiDriver("GLFW");

    if (!glfwInit())
      return window;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* glfwWindow = glfwCreateWindow(
      int(width), int(height), title, nullptr, nullptr);

    if (!glfwWindow)
      return window;

    window.handle = glfwWindow;

  #if defined(_WIN32)
    window.hwnd = glfwGetWin32Window(glfwWindow);
  #else
    window.hwnd = reinterpret_cast<HWND>(glfwWindow);
  #endif
#endif

    return window;
  }

  // Pumps the toolkit event queue. Returns false once the user closes the
  // window, or once the DXVK_EXAMPLE_FRAMES budget is exhausted.
  inline bool processEvents(Window& window) {
    if (window.frameLimit && window.frameCount++ >= window.frameLimit)
      return false;

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
    return !glfwWindowShouldClose(static_cast<GLFWwindow*>(window.handle));
#endif
  }

  inline void destroyWindow(const Window& window) {
    if (!window.handle)
      return;

#if defined(EXAMPLE_WSI_SDL3) || defined(EXAMPLE_WSI_SDL2)
    SDL_DestroyWindow(static_cast<SDL_Window*>(window.handle));
    SDL_Quit();
#elif defined(EXAMPLE_WSI_GLFW)
    glfwDestroyWindow(static_cast<GLFWwindow*>(window.handle));
    glfwTerminate();
#endif
  }

}
