Standard cmake with targets client and server.

Tested on Linux and Windows, though it's currently configured for building on Windows with vcpkg.

You'll need the flatbuffers compiler to build the schemas in fbs/

Required Dependencies for building are the following 'development' packages:
- asio
- glm
- flatbuffers
- curl
- boost
- glfw
- glew
- nuklear
- sqlite3

NOTE: 
The client is going to expect path info as the first argument 
if the "app directory" containing shaders and images is not the working directory from which the exectuable is called.
Both relative and absolute path info should work.
e.g. ./client ../client
