Standard cmake with targets client and server.

Tested on Linux + Windows. You'll have to figure out how to generate the flatbuffer files, and there will be other problems too. A nice easy build experience is a work in progress (at this time it "works on my machine")

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
e.g. ./client ../
