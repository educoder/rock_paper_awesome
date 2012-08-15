/*
  DNS and DHCP-based Web client
 
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Base64.h>
#include <string.h>
#include <stdarg.h>
#include <avr/pgmspace.h>
#include <XMPPClient.h>


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x73, 0x13 };
char serverName[] = "badger.encorelab.org";
char userName[] = "chicago";
char pass[] = "chicago";
char resource[] = "arduino";


// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
XMPPClient xmppClient;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
 
  // Connect to XMPP server
  Serial.println("Connecting to XMPP server...");
   if (xmppClient.xmppLogin(serverName, userName, pass, resource, mac)) {
    Serial.print("Logged in to ");
    Serial.println(serverName);
  } 
  else {
    Serial.print("Impossible to login to ");
    Serial.println(serverName);
  }
}



void loop() {  
  // if there are incoming bytes available 
  // from the server, read them and print them:
//  if (xmppClient.available()) {
//    char c = client.read();
//    Serial.print(c);
//  }

  // if the server's disconnected, stop the client:
//  if (!xmppClient.connected()) {
//    Serial.println();
//    Serial.println("disconnecting.");
//    client.stop();
//
//    // do nothing forevermore:
//    while(true);
//  }
}

