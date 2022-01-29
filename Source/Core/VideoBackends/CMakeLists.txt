add_subdirectory(OGL)
add_subdirectory(Software)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_subdirectory(DX9)
  add_subdirectory(DX11)
  add_subdirectory(D3D12)
endif()

add_subdirectory(Vulkan)

# TODO: Add other backends here!
