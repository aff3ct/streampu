## Socket
Sockets are used to communicate data between [tasks](module & task.md) . There is 3 diffrent types of sockets
		
		- Socket_IN : Getting input data.
		- Socket_OUT : Send ouput data.
		- Socket_Forward : Both In and Out socket, it gets input data and transmits output.

### Attributes

`socket_t type :` Used for  to define the type of the socket.
`std::string name` : Custom name for socket.
`std::type_index datatype` : Type of data exchanged.
`void* dataptr` : Pointer to the data of the socket.
`std::vector<Socket*> bound_sockets` :  Used for both output and forward sockets, it saves the bound sockets to the current one. This vector is empty for input socket.
`Socket* bound_socket` : Used for both input and forward sockets, it saves the unique socket bound to the current one. This pointer is null for output socket.

### Methods
The most important functions of the socket class is the bind and unbind functions.

```cpp
void  bind(Socket  &s_out, const  int  priority  =  -1)
```
 This function is used to connect sockets with each other, it can be called by an input or forward socket and take as parameter an output or forward socket. The function get caller `dataptr` and make it point to `s_out dataptr`.

```cpp
void  unbind(Socket  &s_out, const  int  priority  =  -1)
```
 This function is used to disconnect sockets with each other, the `s_out` must be binded to the caller socket. 

  


