# Server communicating by HTTP protocol 

HTTP Server running on linux (ubuntu 20.04 LTS).

## About program

Server is providing various information about system, is listening to client on selecte port and is able to process three main requests and returns are in text/plain form:
1. GET /hostname  - program returns hostname of system where server is running
2. GET /cpu-name  - program returns name of system's cpu
3. GET /load      - program returns cpu usage
HTML Server could be launched from internet explorer, client console aplication and from terminal by commands wget and curl.
Program needs one parameter - PORT number.

### Creating executable file

```
$ make hinfosvc
```

## Usage

```
$ ./hinfosvc PORT_NUMBER
```
This command will run program and then we can access program through internet explorer, terminal or client application.
e.g. running through internet explorer:
```
$ localhost:PORT_NUMBER/hostname
$ localhost:PORT_NUMBER/cpu-name
$ localhost:PORT_NUMBER/load
```
PORT_NUMBER is in range <0, 65535>

### Author

Adam Dzurilla, xdzuri00
