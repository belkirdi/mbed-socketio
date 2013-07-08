#include "SocketIO.h"

// constructor
SocketIO::SocketIO(char * myurl) {
    // prepare the sessionkey URL
    this->prepareSessionKeyURL(myurl, DEFAULT_VERSION);
}

// constructor
SocketIO::SocketIO(char * myurl, int myversion) {
    // prepare the sessionkey URL
    this->prepareSessionKeyURL(myurl, myversion);    
}

// connect to the SocketIO server
bool SocketIO::connect() {
    bool connected = this->is_connected();
    
    // make sure we are not already connected
    if (connected == false) {
        // first we must acquire the session key
        bool connected = this->acquireSessionKey();
        
        // next we must create a new URL to connect to with the session key
        if (connected == true) {
            // create our session URL
            this->prepareSessionURL();
        
            // allocate our websocket endpoint
            this->ws = new Websocket(this->url_session);
            
            // connect!
            connected = this->attemptWebSocketConnect();
        }
    }
    
    // return our connection status
    return connected;
}

// gracefully disconnect from the SocketIO server
bool SocketIO::close() {
    bool closed = false;
  
    // if we are connected lets disconnect
    if (this->ws != NULL) {
        this->ws->send("0::");
        this->ws->close();
        closed = this->ws->is_connected();
    }
    
    // now clean up memory 
    if (this->ws != NULL) delete this->ws;
    if (this->url_session != NULL) delete this->url_session;
    if (this->url_session_key != NULL) delete this->url_session_key;
    if (this->session_key != NULL) delete this->session_key;
    if (this->ws_channel != NULL) delete this->ws_channel;
    
    // return our status
    return closed;
}

// emit a message (broadcast) to the SocketIO server
int SocketIO::emit(char *name, char * args) {
    int bytesSent = 0;
    
    // only emit if we are connected
    if (this->is_connected() == true) {
        // prepare the JSON message for SocketIO
        char buffer[256];
        char *json = this->prepareSocketIOJSONMessage(name,args,buffer);
        
        // send a heartbeat
        this->ws->send("2:::");
        
        // send the message
        bytesSent = this->ws->send(json);
    }
    
    return bytesSent;
}

// read a message from the SocketIO server
bool SocketIO::read(char * message) {
    bool didRead = false;
    
    // only read if we are connected
    if (this->is_connected() == true && message != NULL) {        
        // attempt a read
        didRead = this->ws->read(message);        
    }
    
    // return our status
    return didRead;
}

// Are we connected?
bool SocketIO::is_connected() {
    bool isConnected = false;
    
    // if we have an endpoint - get its connection status
    if (this->ws != NULL) isConnected = this->ws->is_connected();
    
    return isConnected;
}

// Private Methods

// prepare a SocketIO compatible JSON packet
char *SocketIO::prepareSocketIOJSONMessage(char *name, char *args, char *buffer) {
    sprintf(buffer,"5:::{\"name\": \"%s\", \"args\": %s}",name, args);
    return buffer;
}

// prepare the session URL
void SocketIO::prepareSessionURL() {
    // allocate the buffer
    this->url_session = new char[256];
    
    // fill the buffer
    sprintf(this->url_session,"ws://%s/socket.io/%d/%s",this->url,this->version,this->session_key);
}

// attempt a connection via websockets
bool SocketIO::attemptWebSocketConnect() {
    // attempt connect
    int status = 0;
    for(int i=0;i<10;++i) {
        status = this->ws->connect();
        if (status != 0) i = 10;
    }
    
    // set the socket.io connect command
    if (this->ws->is_connected()) this->ws->send("1::");
    
    // return connection status
    return this->is_connected();
}    

// prepare the session key URL
void SocketIO::prepareSessionKeyURL(char *myurl, int myversion) {
    // save the base URL
    this->url = myurl;
    
    // set our version
    this->version = myversion;
    
    // build out the session key URL
    this->url_session_key = new char[256];
    time_t seconds = time(NULL);
    sprintf(this->url_session_key,"http://%s/socket.io/%d/?t=%d",this->url,this->version,seconds);
    
    // setup the session key and channel buffers
    this->session_key = new char[128];
    this->ws_channel = new char[128];
}

// parse the socket.io session key
void SocketIO::parseSessionKey(char *response, char *sessionkey, char *channel) {
     int val1 = 0;
     int val2 = 0;
     
     // convert ":" to " "
     for(int i=0;i<strlen(response);++i) if (response[i] == ':') response[i] = ' ';
     
     // format:  Session_ID YY ZZ CHANNEL
     char t_sessionkey[128];
     sscanf(response,"%s %d %d %s",t_sessionkey,&val1,&val2,channel);
     
     // create
     sprintf(sessionkey,"%s/%s",channel,t_sessionkey);            
}

// acquire the session key for our session
bool SocketIO::acquireSessionKey() {
    bool haveKey = false;
    HTTPClient http;
    char response[513];
    
    // make sure we have buffers
    if (this->session_key != NULL && this->ws_channel != NULL) {
        // request our session key
        int nread = http.get(this->url_session_key,response,512);
        
        // parse the session key
        if (!nread)         
             // parse into the session key
             this->parseSessionKey(response,this->session_key,this->ws_channel);
        
        // update our status
        if (strlen(this->session_key) > 0) haveKey = true;
    }
        
    // return status
    return haveKey;
}