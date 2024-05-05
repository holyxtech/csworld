Standard cmake with targets CsWorldClient and CsWorldServer for client and server respectively.

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
CsWorldClient is going to expect path info as the first argument 
if the "app directory" containing shaders and images is not the working directory from which the exectuable is called.
Both relative and absolute path info should work.
e.g. ./CsWorldClient ../
