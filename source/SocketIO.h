/**
* @author Doug Anson
*
* @section LICENSE
*
* Copyright (c) 2013 mbed
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @section DESCRIPTION
*    Simple SocketIO client library
*
*/

#ifndef SOCKETIO_H
#define SOCKETIO_H

#include "mbed.h"

// Default SocketIO version
#define DEFAULT_VERSION         1

// Default SocketIO message length (suggestion only...)
#define SOCKETIO_MESSAGE_LENGTH 512

// HTTP support for session key retrieval
#include "HTTPClient.h"

// WebSocket layer support
#include "Websocket.h"

/** SocketIO client Class.
 *
 * Example (ethernet network):
 * @code
 * #include "mbed.h"
 * #include "WiflyInterface.h"
 * #include "SocketIO.h"
 * 
 * // our wifi interface
 * WiflyInterface wifly(p9, p10, p30, p29, "mysid", "mypw", WPA); 
 * 
 * int main() {
 *    wifly.init(); //Use DHCP
 *    wifly.connect();
 *    printf("IP Address is %s\n\r", wifly.getIPAddress());
 *   
 *    SocketIO socketio("myapp.herokuapp.com");
 *    socketio.connect();
 *   
 *    char recv[256];
 *    while (1) {
 *       int res = socketio.emit("mesage_name","[\"SocketIO Hello World!\"]");
 *        if (socketio.read(recv)) {
 *            printf("rcv: %s\r\n", recv);
 *        }
 *        wait(0.1);
 *    }
 * }
 * @endcode
 */
 
class SocketIO
{
    public:
        /**
        * Constructor
        *
        * @param url The SocketIO url in the form "www.example.com:[port]" (by default: port = 80) - i.e. just the endpoint name
        */
        SocketIO(char * url);
        
        /**
        * Constructor
        *
        * @param url The SocketIO url in the form "www.example.com:[port]" (by default: port = 80) - i.e. just the endpoint name
        * @param version The SocketIO version for the session URL (by default version = 1)
        */
        SocketIO(char * url, int version);

        /**
        * Connect to the SocketIO url
        *
        *@return true if the connection is established, false otherwise
        */
        bool connect();

        /**
        * Emit (Broadcast) a socket.io message to the SocketIO server
        *
        * Socket.IO message Format (JSON): { "name": <name>, "args":<args> }
        *       name: the "name" of the socket.io message
        *       args: the argument(s) (must always be a JSON array) of the message. Example: "[ \"foo\", {\"bar\": \"none\"}]" 
        *
        * @param name "name" of the socket.io message to broadcast
        * @param args argument string to be sent ( must be in a JSON array format. Example: "[ \"foo\", {\"bar\": \"none\"}]" )
        *
        * @returns the number of bytes sent
        */
        int emit(char * name, char * args);

        /**
        * Read a SocketIO message
        *
        * @param message pointer to the string to be read (null if drop frame)
        *
        * @return true if a SocketIO frame has been read
        */
        bool read(char * message);

        /**
        * To see if there is a SocketIO connection active
        *
        * @return true if there is a connection active
        */
        bool is_connected();

        /**
        * Close the SocketIO connection
        *
        * @return true if the connection has been closed, false otherwise
        */
        bool close();

    protected:
        Websocket   *ws;                // websocket endpoint
        
    private:
        // Variables
        int         version;            // default socket.io version
        char        *url;               // base URL endpoint to connect to. Example: "myapp.herokuapp.com"
        char        *url_session_key;   // generated session key URL - used to extract the session key 
        char        *url_session;       // session URL - specific socket.io session to bind to        
        char        *session_key;       // our session key
        char        *ws_channel;        // our websocket channel for the session
        
        // Methods
        char *prepareSocketIOJSONMessage(char *name, char *args, char *buffer);
        void prepareSessionURL();
        bool attemptWebSocketConnect();
        void parseSessionKey(char *response, char *sessionkey, char *ws_channel);
        bool acquireSessionKey();
        void prepareSessionKeyURL(char *myurl, int myversion);
        
        
};

#endif