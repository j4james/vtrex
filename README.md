VT-Rex
======

![Screenshot](screenshot.png)

This is an implementation of the Google Chrome [dinosaur game], designed to
be played on a DEC VT terminal. It requires at least a VT420 (or something of
comparable functionality), but a VT525 is best if you want color and sound
effects.

[dinosaur game]: https://en.wikipedia.org/wiki/Dinosaur_Game


Download
--------

The latest binaries can be found on GitHub at the following url:

https://github.com/j4james/vtrex/releases/latest

For Linux download `vtrex`, and for Windows download `vtrex.exe`.


Build Instructions
------------------

If you want to build this yourself, you'll need [CMake] version 3.15 or later
and a C++ compiler supporting C++20 or later.

1. Download or clone the source:  
   `git clone https://github.com/j4james/vtrex.git`

2. Change into the build directory:  
   `cd vtrex/build`

3. Generate the build project:  
   `cmake -D CMAKE_BUILD_TYPE=Release ..`

4. Start the build:  
   `cmake --build . --config Release`

[CMake]: https://cmake.org/


License
-------

The VT-Rex source code and binaries are released under the MIT License. See
the [LICENSE] file for full license details.

[LICENSE]: LICENSE.txt
