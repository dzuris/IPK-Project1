# IPK Project 1

HTML Server running on linux (ubuntu 20.04 LTS)

## About program

Program is listening to client and his request, program is able to process three main requests and returns are in text/plain style:
1. GET /hostname  - program returns hostname of system where server is running
2. GET /cpu-name  - program returns name of system's cpu
3. GET /load      - program returns cpu usage

### Installation

Program work with gcc compiler and is transable by Makefile with command "make hinfosvc"

### Usage

Program needs one parameter - PORT number.
Program is running by console with command e.g. "./hinfosvc 8080"

### Author

Adam Dzurilla, xdzuri00
