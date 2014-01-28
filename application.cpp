/**
 ******************************************************************************
 * @file    application.cpp
 * @authors Philip Henzler
 * @version V0.0.1
 * @date    28-January-2014
 * @brief   Sonos Controller for Spark
 ******************************************************************************
  Copyright (c) 2013 Philip Henzler.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
*/


// REMOVE THIS IF BUILDING VIA spark.io/build
/* Includes ------------------------------------------------------------------*/  
#include "application.h"


#define SERIALDEBUG 1

// Make the parsing part output some informations via Serial interface
// #define SERIALDEBUGPARSER 1

// Make the volume get function more verbose
// #define SERIALDEBUGGETVOL 1

// Make response versboe
// #define RESPONSEDEBUG 1



/* Sonos ------------------------------------------------------------------*/

// Sonos
int getVolume();
int getMute();

void setVolume(int volVal);
void setMute(int mutVal);


// General
String getResponse();
String elementContent(String input, String element);





/* Vars ------------------------------------------------------------------*/

TCPClient client;
int incomingByte = 0;   // for incoming serial data

const int __SONOS_PORT = 1400;
byte sonos_ip[] = { 10, 0, 6, 118 };

// Change last byte of the sonos_ip
void setSonosA() {
    sonos_ip[3] = 114;
}

void setSonosB() {
    sonos_ip[3] = 118;
}



/* Initial ------------------------------------------------------------------*/

void setup()
{
    Serial.begin(9600);
    delay(1000);
}



/* Good old Loop -------------------------------------------------------------*/


void loop()
{
    
    if (Serial.available() > 0) {
        incomingByte = Serial.read();
        
        // for testing...
        //if(incomingByte != 13) {
            //Serial.print("I received: ");
            //Serial.println(incomingByte, DEC);
        //}
        
        // [A] pressed -------------------------
        if (incomingByte == 97) {
            Serial.println("Selected Sonos A");
            setSonosA();
            
        // [B] pressed -------------------------
        } else if (incomingByte == 98) {
            Serial.println("Selected Sonos B");
            setSonosB();
            
        // [M] pressed -------------------------
        } else if (incomingByte == 109) {
            int val = getMute();
            
            if(val) {
                Serial.println("Set Mute to 0");
                setMute(0);
            } else {
                Serial.println("Set Mute to 1");
                setMute(1);
            }
            
        // [U] pressed -------------------------
        } else if (incomingByte == 117) {
            int newvol = getVolume()+5;
            
            if (newvol <=100)
            {
                Serial.print("Set Volume to: ");
                Serial.println(newvol);
                
                setVolume(newvol);
            } else {
                Serial.println("Set Volume to 100");
                setVolume(100);
            }
        
        // [J] pressed -------------------------
        } else if (incomingByte == 106) {
            int newvol = getVolume()-5;
            
            if (newvol >= 0)
            {
                Serial.print("Set Volume to: ");
                Serial.println(newvol);
                
                setVolume(newvol);
            } else {
                
                Serial.println("Set Volume to 0");
                setVolume(0);
            }
        }
    
    }
}



// send the header
void openMessage(TCPClient tcc, const char* payload, const char* soapAction) {

    char _buffer[81] = {0};
    
    tcc.println("POST /MediaRenderer/RenderingControl/Control HTTP/1.1");
    
    snprintf(_buffer, 80, "Host: %d.%d.%d.%d:%d", sonos_ip[0], sonos_ip[1], sonos_ip[2], sonos_ip[3], __SONOS_PORT);
    tcc.println(_buffer);
    
    snprintf(_buffer, 80, "Content-Length: %d" , strlen(payload));
    tcc.println(_buffer);
    
    snprintf(_buffer, 80, "SOAPACTION: urn:schemas-upnp-org:service:RenderingControl:1#%s",  soapAction);
    tcc.println(_buffer);

    tcc.println("Accept: */*");
    tcc.println();
    
}


// create the soap request
// some parts are redundant in every call, so keep it here
void packSoap(char *ret, int bufflen,  const char* content)
{
    char soap_open[160];
    memset(soap_open, 0, 160);
    
    // Constant parts of the request
    // This shall be used instead of the buffer above!!
    // const char *soap_open = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>";
    const char *soap_close = "</s:Body></s:Envelope>";

    
    // Following: A little stunt, to build it at spark.io/build
    // background: spark.io/build does not acceppt double slashes forward -> so it wont build with "htt://..."
    // workaround: splitting the string and rejoining it... ugly, but.. well...
    
    snprintf(soap_open, 159, "%s%s%s", "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http:/", "/schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http:/", "/schemas.xmlsoap.org/soap/encoding/\"><s:Body>");
    
    
    // make buffer zero
    memset(ret, 0, bufflen);
    
    // -1 because wie use strlen later in the code, so with zerosetting above we can be sure that at least last position is 0
    snprintf(ret, bufflen-1, "%s%s%s", soap_open, content, soap_close);

}



// # sonos control GETTERS  --------------------------------------------------


int getVolume() {
    if (client.connect(sonos_ip, __SONOS_PORT))
    {
        char _payload[400] = {0};
    
        const char *_content = "<u:GetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><Channel>Master</Channel></u:GetVolume>";
        packSoap(_payload, 400, _content);
        
        openMessage(client, _payload, "GetVolume");
        client.flush();
        client.println(_payload);

        //response is something like: ...schemas-upnp-org:service:RenderingControl:1"><CurrentVolume>29</CurrentVolume></u:GetVolumeResponse></s:Body></s:Envelope>
        String sonosResponse = getResponse();
        
        client.flush();
        client.stop();
    
        // Parse the response for the CurrentVolume Elements content
        String volStr = elementContent(sonosResponse, "CurrentVolume");
        
        #ifdef SERIALDEBUGGETVOL
            Serial.print("[INF] VolString: '");
            Serial.print(volStr);
            Serial.print("'");
        #endif // SERIALDEBUG
        
        // Convert Volume String to Integer
        if (volStr != NULL){
            return volStr.toInt();
        }
        
    } else {
        
        #ifdef SERIALDEBUG
            Serial.println("[ERR] Connection was not possible.");
        #endif // SERIALDEBUG
        
    }
    
    return -1;
}



// return mute state
int getMute() {
    
    if (client.connect(sonos_ip, __SONOS_PORT))
    {
    char _payload[400] = {0};

    const char *_content = "<u:GetMute xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><Channel>Master</Channel></u:GetMute>";
    packSoap(_payload, 400, _content);

    openMessage(client, _payload, "GetMute");
    client.flush();
    client.println(_payload);

    // response looks like: ....np-org:service:RenderingControl:1"><CurrentMute>0</CurrentMute></u:GetMuteResponse></s:Body></s:Envelope>
    String sonosResponse = getResponse();

    client.flush();
    client.stop();
        
    // parse for CurrentMute in the response
    String muteStr = elementContent(sonosResponse, "CurrentMute");

        
    // Convert Volume String to Integer
    if (muteStr != NULL){
        return muteStr.toInt();
    }
    
    } else {
        
        #ifdef SERIALDEBUG
            Serial.println("[ERR] Connection was not possible.");
        #endif // SERIALDEBUG
        
    }
    
    return -1;
    
}



// # sonos control SETTERS  --------------------------------------------------


// Set the volume to a specific value (0 to 100)
void setVolume(int volVal) {
    if (client.connect(sonos_ip, __SONOS_PORT))
    {
        char _payload[400] = {0};
        char _buffer[180] = {0};
        
        snprintf(_buffer, 179, "<u:SetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><Channel>Master</Channel><DesiredVolume>%d</DesiredVolume></u:SetVolume>", volVal);
        
        packSoap(_payload, 400, _buffer);
        
        
        openMessage(client, _payload, "SetVolume");
        
        client.println(_payload);
        client.println();
        client.println();

        // not really interested in the response ;)
        client.flush();
        client.stop();
        
    } else {
        
        #ifdef SERIALDEBUG
            Serial.println("[ERR] Connection was not possible.");
        #endif // SERIALDEBUG
    }
}


// Set mute (0 or 1)
void setMute(int mutVal) {
    if (client.connect(sonos_ip, __SONOS_PORT))
    {
        char _payload[400] = {0};
        char _buffer[180] = {0};
        
        snprintf(_buffer, 179, "<u:SetMute xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><Channel>Master</Channel><DesiredMute>%d</DesiredMute></u:SetMute>", mutVal);
        
        packSoap(_payload, 400, _buffer);
        
        
        openMessage(client, _payload, "SetMute");
        
        client.println(_payload);
        client.println();
        client.println();
        
        // not really interested in the response ;)
        client.flush();
        client.stop();
        
    } else {
        
    #ifdef SERIALDEBUG
        Serial.println("[ERR] Connection was not possible.");
    #endif // SERIALDEBUG
    }
}








// # receiving of the response -------------------------------


String getResponse() {
    char rx_buffer[2048];
    int bufflen=2048;
    int recLen = 0;
    
    memset(rx_buffer, 0, sizeof(rx_buffer));
    
    
    // wait until data is available, max 5 seconds
    uint32_t sT = millis();
    while(!client.available() && (millis() - sT) < 5000) {
        //delay(1);
    }
    
    recLen = client.available();
    
    if (recLen > 0)
    {
        // read from client to the buffer, for length of bytes
        for (int i = 0;  i  < recLen && i < bufflen; i++) {
            char rec = client.read();
            if (rec == -1) {
                break;
            }
            rx_buffer[i] = rec;
        }
    } else {
        return NULL;
    }
    
    
    // make string from buffer
    String rx_string(rx_buffer);
    
    #ifdef RESPONSEDEBUG
        Serial.print("[INF] response length: ");
        Serial.println(rx_string.length());
    #endif

    return rx_string;
    
}


// # simple parsing of the responses ------------------------------
//
// find <element>X</element> and return X as a String
//

String elementContent(String input, String element)
{
    // make open and close string
    String elementOpen = "<" + element + ">";
    String elementClose = "</" + element + ">";
    
    // get positions of open and close
    int openPos = input.indexOf(elementOpen);
    int closePos = input.indexOf(elementClose);
    
    #ifdef SERIALDEBUGPARSER
        Serial.print("[INF] Input length: ");
        Serial.println(input.length());
        Serial.println("[INF] Elements:");
        Serial.println(elementOpen);
        Serial.println(elementClose);
        Serial.println("[INF] Positions:");
        Serial.println(openPos);
        Serial.println(closePos);
    #endif // SERIALDEBUG
    
	
    // verify if open and close can be found in the input
    if (openPos == -1 || closePos == -1) {
		Serial.println("[ERR] can not find element in input");
		return NULL;
	}
    
    // idx: starts at index of element + length of element + tags
    int idx = openPos + elementOpen.length();
 
    // check if length is above 0
    if (closePos - idx > 0) {
        #ifdef SERIALDEBUGPARSER
            Serial.print("[INF] Output length: ");
            Serial.println((input.substring(idx,  closePos)).length());
        #endif
        return input.substring(idx,  closePos);

    } else {
        #ifdef SERIALDEBUGPARSER
            Serial.println("[ERR] idx seems wrong");
        #endif
        return NULL;
        
    }
    
}




