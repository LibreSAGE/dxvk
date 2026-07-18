# Minimal DXVK examples. Each D3D API gets a 'clear' and a 'triangle' example,
# built once for every available windowing backend (SDL2, SDL3, GLFW). Only
# built when -Dbuild_examples=true.
#
# On DXVK Native the toolkit dependencies double as the WSI backends and are
# already resolved by the top-level meson.build. On Windows DXVK itself always
# uses the Win32 WSI, so the toolkits are looked up here purely for the
# examples and are optional -- we build whichever ones are available.

if platform == 'windows'
  example_sdl3 = dependency('sdl3',  required: false)
  example_sdl2 = dependency('sdl2',  required: false)
  example_glfw = dependency('glfw3', required: false)
else
  example_sdl3 = lib_sdl3
  example_sdl2 = lib_sdl2
  example_glfw = lib_glfw
endif

example_wsi_backends = []
if example_sdl3.found()
  example_wsi_backends += [{ 'suffix': 'sdl3', 'define': '-DEXAMPLE_WSI_SDL3', 'dep': example_sdl3 }]
endif
if example_sdl2.found()
  example_wsi_backends += [{ 'suffix': 'sdl2', 'define': '-DEXAMPLE_WSI_SDL2', 'dep': example_sdl2 }]
endif
if example_glfw.found()
  example_wsi_backends += [{ 'suffix': 'glfw', 'define': '-DEXAMPLE_WSI_GLFW', 'dep': example_glfw }]
endif

if example_wsi_backends.length() == 0
  error('build_examples requires at least one of SDL3, SDL2 or GLFW')
endif

# One entry per (D3D API, example) pair: the source file and the matching D3D
# dependency. Each API is gated on the corresponding enable_* option so we only
# build what exists. Every API gets a 'clear' and a 'triangle' example.
example_progs = []
if get_option('enable_d3d8')
  example_progs += [{ 'name': 'd3d8_clear',     'src': 'd3d8/d3d8_clear.cpp',        'dep': d3d8_dep }]
  example_progs += [{ 'name': 'd3d8_triangle',  'src': 'd3d8/d3d8_triangle.cpp',     'dep': d3d8_dep }]
endif
if get_option('enable_d3d9')
  example_progs += [{ 'name': 'd3d9_clear',     'src': 'd3d9/d3d9_clear.cpp',        'dep': d3d9_dep }]
  example_progs += [{ 'name': 'd3d9_triangle',  'src': 'd3d9/d3d9_triangle.cpp',     'dep': d3d9_dep }]
endif
if get_option('enable_d3d10')
  example_progs += [{ 'name': 'd3d10_clear',    'src': 'd3d10/d3d10_clear.cpp',      'dep': [ d3d10_core_dep, dxgi_dep ] }]
  example_progs += [{ 'name': 'd3d10_triangle', 'src': 'd3d10/d3d10_triangle.cpp',   'dep': [ d3d10_core_dep, dxgi_dep ] }]
endif
if get_option('enable_d3d11')
  example_progs += [{ 'name': 'd3d11_clear',    'src': 'd3d11/d3d11_clear.cpp',      'dep': [ d3d11_dep, dxgi_dep ] }]
  example_progs += [{ 'name': 'd3d11_triangle', 'src': 'd3d11/d3d11_triangle.cpp',   'dep': [ d3d11_dep, dxgi_dep ] }]
endif

foreach prog : example_progs
  foreach wsi : example_wsi_backends
    executable('example_' + prog['name'] + '_' + wsi['suffix'], prog['src'],
      dependencies        : [ prog['dep'], wsi['dep'] ],
      include_directories : dxvk_include_path,
      cpp_args            : [ wsi['define'] ],
      install             : false,
    )
  endforeach
endforeach
