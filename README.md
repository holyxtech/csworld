Standard cmake with targets client and server.

Tested under linux only so far.

Required Dependencies for building are the following 'development' packages:
- asio
- glm
- flatbuffers
- curl
- boost
- glfw
- glew

The build should prompt you if any of the packages required are not installed.

NOTE: 
The client is going to expect path info as the first argument 
if the "app directory" containing shaders and images is not the working directory from which the exectuable is called.
Both relative and absolute path info should work.
e.g. ./client ../
