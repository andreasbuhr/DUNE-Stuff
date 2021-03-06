# This file is part of the dune-stuff project:
#   http://users.dune-project.org/projects/dune-stuff
# Copyright holders: Rene Milk, Felix Schindler
# License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

set(lib_dune_stuff_sources common/filesystem.cc 
  common/color.cc 
  common/logging.cc 
  common/logstreams.cc
  common/profiler.cc
  common/parameter/configcontainer.cc
  common/parameter/tree.cc
  common/signals.cc
  common/math.cc
  common/threadmanager.cc
  fem/femeoc.cc
  grid/fakeentity.cc 
  functions/interfaces.cc 
  functions/checkerboard.cc
  functions/constant.cc
  functions/expression/mathexpr.cc
  functions/expression.cc
  functions/spe10.cc
  functions.cc
  la/container/pattern.cc )

dune_add_library("dunestuff" ${lib_dune_stuff_sources}
  ADD_LIBS ${DUNE_LIBS})
target_link_dune_default_libraries(dunestuff)
if(dune-grid_FOUND)
  add_dune_alugrid_flags(dunestuff)
endif(dune-grid_FOUND)

add_analyze(${lib_dune_stuff_sources})

add_subdirectory(test EXCLUDE_FROM_ALL)
