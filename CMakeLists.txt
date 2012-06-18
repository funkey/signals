# find all files constituting this library
file(GLOB signalsfiles
  *.cpp)

# this is a library
add_library(signals ${signalsfiles})
