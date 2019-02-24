# ZiaModuleAPISpec

API specification for "zia" server modules

## Installation

1. `clone` this repository in your project in the `lib/` directory
2. Add `add_subdirectory(lib/ZiaModuleAPISpec)` into your cmake
3. Add `target_link_libraries(<target_name> ${CONAN_LIBS_BOOST} ZiaModuleAPISpec)` to each target using the API spec.
4. Eventually `pull` this repository to update the API specification

Or add `lib/ZiaModuleAPISpec/include` to your include directory path.

## Dependencies

- C++11 STL
- Boost Asio

## Module implementation guide

### Interface

A module is contained in a dynamic library (*.so on linux / *.dll on windows).

This library must export a free function named `factory` that creates an instance of the Module class. This is mandatory in order for the server to load your module, as class definitions can not be exported through dynamic libraries.

This library must also export a free function named `recycler` that will take care of the destruction of the Module instance.

Example: `samples/modules/log/factory.cpp`

### Module class

A module is a child-class of the `Zia::API::Module` abstract class. 

The constructor and destructor should be used to allocate & deallocate resources used by the module. Those will be called when the module is loaded and unloaded, respectively.

This class may define *activation hooks*: these hooks are related to the module life-cycle. The server calls those hooks when appropriate.
- **onActivate**: Called when the module is activated. The server configuration is provided in the hooks arguments. The module should initialize its internal data, and save relevant configuration inside its instance.
- **onDeactivate**: Called when the module is deactivated.
- **onConfigChange**: Called when the server configuration was changed. The new server configuration is provided in the hooks arguments. The module should update the relevant configuration inside its instance.

The module class must also define a *request handler* factory. This method must create an instance of a child-class of the `RequestHanlder` abstract class. For each incoming request, a new instance of the request handler will be created. This permits the request handler to persist request-bound data during the request processing to use in different request hooks. An instance of the module class should be passed to the request handler instance in order to access the module data.

Example: `samples/modules/log/Module.hpp`

### Request handler

A module must define a child-class of the `RequestHandler` abstract class.

For each incoming request, the server creates a new instance of the request handler. The request handler will be used for one request only. This handler will expose hooks that will be called by the server accordingly.

The constructor and destructor should be used to allocate & deallocate resources used by the request handler. Those will be called when the request initialized and uninitialized, respectively.

The request handler may define *connection hooks*: these hooks are related to the establishment of the tcp connection.
- **onConnectionStart**: Called when the connection is opened
- **onConnectionEnd**: Called when the connection is about to be closed
- **onConnectionRead**: Asks the module to read data from the socket
- **onConnectionWrite**: Asks the module to write data to the socket

The request handler may define *request hooks*: these hooks are related to the request handling & response generation.
- **onBeforeRequest**: Called before calling **onRequest**, in order for a module to alter the incoming request.
- **onRequest**: Called in order to process the request and generate a response.
- **onRequestError**: Called when an error occurred in **onBeforeRequest** or **onRequest**, in order to generate a error page.
- **onResponse**: Called after the response is generated by **onRequest** or **onRequestError**, in order for a module to alter the outgoing response.

Example: `samples/modules/log/RequestHandler.hpp`

### Hook result

A request handler hook must return a hook result. A hook result can be:
- **Ok**: Tells the server that this hook was processed by the module.
- **Declined**: Tells the server that this hook wasn't processed by the module.
- **Any other positive number**: Tells the server that an error occurred while processing this hook. A HTTP status code can be provided as the hook result, in order to generate a specific error page.

### Server behaviour

In order to call the module hooks, the server creates a list of the module instances and reorders it according to the user configuration. A module at the start of the list will be called first. The server will then iterate through all modules and call their hooks, one by one.

When a hook returns **Ok**, the server must stop the processing of this hook.
When a hook returns **Decline**, the server must continue the processing of this hooks, until no hooks are left to call.
When a hook return **Error**, the server must stop the processing of this hook. It can then call the **onRequestError** hook if applicable to generate a response and continue.

Example:
- **Starting**
- server starts
- server loads `module.dll`
- `module->onActivate`
- `module->onConfigChange`
- **On request**
- server accepts new connection
- `module->newRequestHandler`
- `requesthandler->onConnectionStart`
- `requesthandler->onConnectionRead`
- server parses incoming request
- `requesthandler->onBeforeRequest`
- `requesthandler->onRequest`
- `requesthandler->onResponse`
- server serializes outgoing response
- `requesthandler->onConnectionWrite`
- `requesthandler->onConnectionEnd`
- server closes connection
- **Shutting down**
- `module->onDeactivate`
- server unloads `module.dll`
- server shuts down

## Diving deeper

See the headers in `include/Zia/API` for more details about the different class & structures of this API specification. A class diagram is provided below for easier understanding of the API organization.

An example module is provided in `samples/modules/log`.

## Class diagram (outdated, please refer to the headers)

```
+---------------------+         +------------------------+         +-----------------+   
| MODULE (class)      |         | SERVER CONFIG (struct) |         | PLATFORM (enum) |   
|                     |         |                        |         |                 |   
| Meta-data:          |         | - name                 |    +----| - Linux         |   
| - getName           |    +----| - version              |    |    | - Windows       |   
|                     |    |    | - platform             -----+    | - Macos         |   
| Hooks:              |    |    | - config               |         +-----------------+   
| - onActivate        |    |    | - apispec_version      -----+                          
| - onDeactivate      |    |    +------------------------+    |    +--------------------+
| - onConfigChange    -----+                                  |    | DEFINITIONS (vars) |
|                     |                                       +----|                    |
| Request handler:    |                                            | - VERSION          |
| - newRequestHandler -----+                                       +--------------------+
+---------------------+    |                                                             
           |               |    +-------------------------+         +------------------+ 
+----------|----------+    |    | REQUEST HANDLER (class) |         | REQUEST (struct) | 
|       FACTORY       |    |    |                         |         |                  | 
+---------------------+    |    | Connection hooks:       |         | - method         | 
                           |    | - onReadRequest         |         | - uri            | 
                           |    | - onWriteResponse       |    +----| - protocol       | 
                           +----|                         |    |    | - headers        | 
                                | Request hooks:          |    |    | - body           | 
                                | - onPreRequest          |    |    | - secure         | 
                                | - onRequest             -----+    +------------------+ 
                                | - onRequestError        -----+                         
                                | - onResponse            |    |    +-------------------+
                                +------------|------------+    |    | RESPONSE (struct) |
                                             |                 |    |                   |
                                +------------------------+     |    | - protocol        |
                                | HOOK RESULT (number)   |     +----| - status_code     |
                                |                        |          | - status_message  |
                                | - Ok                   |          | - headers         |
                                | - Declined             |          | - body            |
                                | - Any HTTP status code |          +-------------------+
                                +------------------------+                               
```
