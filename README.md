## CCraft

### Build

#### Windows (MinGW)

    mkdir build && cd build
    cmake .. -G "MinGW Makefiles"
    mingw32-make

#### Linux and MacOS

    mkdir build && cd build
    cmake ..
    make

### Run

#### Client

    ./CCraft

Connect to a server by specifying its IP address and optionally its port (the default is 25565)

    ./CCraft --ip <IP> --port <PORT>
    
#### Server

You can optionally specify the server's port (the default is 25565) 

    ./CCraftServer --port <PORT>