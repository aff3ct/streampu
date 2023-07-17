
## Socket
Sockets are used to communicate data between [tasks](module & task.md). There are 3 different types of sockets : 
- Socket_IN : Getting input data.
- Socket_OUT : Send output data.
- Socket_Forward : Both In and Out socket, it gets input data and transmits output.

### Attributes
```cpp
socket_t type;
```
Used for to define the type of the socket.
```cpp
std::string name;
```
Custom name for socket.
```cpp
std::type_index datatype
```
Type of data exchanged.
```cpp
void* dataptr
```
Pointer to the data of the socket.
```cpp
std::vector<Socket*> bound_sockets;
```
It saves the `input or forward` bound sockets to the current `output or forward` socket. This vector is empty for the input socket.
```cpp
Socket* bound_socket;
```
It saves the `output or forward` unique bound socket to the current `input or forward` socket. This pointer is empty for the output socket.

### Methods
The most important methods of the socket class are the bind and unbind functions.

```cpp
void  bind(Socket  &s_out, const  int  priority = -1)
```

This function is used to connect sockets with each other, it can be called by an input or forward socket and take as parameter an output or forward socket. The function get caller `dataptr` and make it point to `s_out dataptr`. The socket can be bound this way : 

 - `In->Out`				
 - `In->FWD` 						Changer par un dessin de graphe !!!
 - `FWD<->FWD`
 - `FWD->OUT`


  

```cpp
void  unbind(Socket  &s_out, const  int  priority = -1);
```
This function is used to disconnect sockets with each other, the `s_out` must be bound to the caller socket.
