INCLUDE(BuildPlugin)

ADD_DEFINITIONS(-DFFTW3)

INCLUDE_DIRECTORIES(${FFTW3F_INCLUDE_DIRS})

LINK_DIRECTORIES(${FFTW3F_LIBRARY_DIRS})
LINK_LIBRARIES(-lfftw3f)

BUILD_PLUGIN(impulse_convolver ImpulseConvolver.cpp ImpulseConvolverControls.cpp ImpulseConvolverControlDialog.cpp MOCFILES ImpulseConvolverControls.h ImpulseConvolverControlDialog.h EMBEDDED_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
