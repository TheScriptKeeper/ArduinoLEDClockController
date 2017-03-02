/*
 * http://code.google.com/p/arduino-seven-segment/
 *
 *
 * You will need to set your driver and your screen configuration in the line
 * screen.begin("AY0438","8.8|8.8");
 *
 * For best you results you should use a display with 4 digits and a colon.
 *
 * You also need an ethernet shield!
 *
 */

/*

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 17 Sep 2010
 by Tom Igoe
 modified Jan 2012 for SevenSegment library
 by Marcus Mason
 modified 7 Nov 2016
 by Jeremiah Duke

 This code is in the public domain. Customizations have been made to work with the giant LED clock I have developed.

*/
#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>
#include <Ntp.h>
#include <EthernetUDP.h>
#include <SevenSegment.h>

#define CLOCK 3 // Arudino digital 44 -> AY0438 clock
#define DATA 4	 // Arudino digital 46 -> AY0438 data
#define LOAD 45  // Arudino digital 45 -> AY0438 load

#define LOCAL_PORT 8888

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10,4,8,16 }; //SGS Clock IP
//byte ip[] = { 192,168,1,33 }; //Home Clock IP
byte subnet[] = { 255,255,252,0};
byte gateway[] = { 10,4,8,1};
byte dnsM[] = { 10,4,8,254};

// NTP server IP
//byte timeServer[]    = { 10,4,8,223}; // SGS Mac Mini Time Server
//byte timeServer[]    = { 17,253,64,243}; // time.apple.com
byte timeServer[]    = { 10,1,2,67}; // SGS Time Server
//byte timeServer[]    = { 66,175,209,17}; // time.nist.gov
//byte timeServer[] = { 130,149,17,21};    // ntps1-0.cs.tu-berlin.de
//byte timeServer[] = { 192,53,103,108};   // ptbtime1.ptb.de

// Store previous time
time_t prevDisplay = 0;

const int NTP_PACKET_SIZE = 48;

// packet buffer
byte packetBuffer[NTP_PACKET_SIZE];

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Setup seven segment
SevenSegment screen(CLOCK, DATA, LOAD);



void setup(){
        Serial.begin(9600);
	// Set display driver AY0438 and screen definition
	screen.begin("M5451","8.8|8.8.");

	screen.print("Load");
        Serial.println("Setting up...");
	// Begin our networking
	Ethernet.begin(mac,ip,dnsM,gateway,subnet);
	if(Udp.begin(LOCAL_PORT)){
        Serial.println("UDP has began.");
}


	// Tell the Time library where to get time from
	// It also update itself periodically to prevent drift
	setSyncProvider(getNTPTime);

	// Wait until we have a time!
	while(timeStatus() == timeNotSet);

        Serial.println("Setup done");
}

void loop(){

	// update the display only if the time has changed (every second)
	if (now() != prevDisplay) {
		prevDisplay = now();
		digitalClockDisplay();
	}
}

// Display the time!
void digitalClockDisplay(){

	static boolean showColon;
	static char buf[9];
        int tHour; //= hour()-6;
        if (hour() > 12){
                //Subtract 12
                tHour = hour()-12;
                
        }else{
                //Move on
                tHour = hour();
        }
        
        //Daylight Saving time - Comment for Fall back...Un-comment for spring forward 
        //tHour=tHour+1;
        if (tHour == 13){
          tHour = 1;
        }
        //Serial.println(tHour);
        //Serial.println(minute());
        if (tHour >= 10){

	         // Create time string, display a colon or not?
	        if (!showColon){
		        //sprintf(buf, "%02d%02d\0", tHour, minute());
                        sprintf(buf, "%02d%02d\0", tHour, minute());
	        }else{
		        //sprintf(buf, "%02d:%02d\0", tHour, minute());
                        sprintf(buf, "%02d:%02d\0", tHour, minute());
                }
        }else{
          
            if(tHour == 0){
                // Create time string, display a colon or not?
                if (!showColon){
  		        //sprintf(buf, "%02d%02d\0", tHour, minute());
                          sprintf(buf, "12%02d\0", minute());
  	        }else{
  		        //sprintf(buf, "%02d:%02d\0", tHour, minute());
                          sprintf(buf, "12:%02d\0", minute());
                  }
              
            }else{
                 // Create time string, display a colon or not?
                if (!showColon){
  		        //sprintf(buf, "%02d%02d\0", tHour, minute());
                          sprintf(buf, "% 2d%02d\0", tHour, minute());
  	        }else{
  		        //sprintf(buf, "%02d:%02d\0", tHour, minute());
                          sprintf(buf, "% 2d:%02d\0", tHour, minute());
                  }
              
              
            }
                 
	        
          
          
        }


	// Display the time on our screen
	screen.print(buf);
        
	// Toggle display of colon
	showColon = !showColon;

        //Serial.println("Got time");
}


// Retrieve NTP time
time_t getNTPTime(){

	// send an NTP packet to a time server
	sendNTPpacket(timeServer);
        Serial.println("Sent packet");
	// wait to see if a reply is available
	delay(1000);
        //Serial.println(Udp.parsePacket());
	if (Udp.parsePacket()) {
                Serial.println("UDP is available");
		Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

		//the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;

		// subtract seventy years:
		unsigned long epoch = secsSince1900 - seventyYears;

                // print the hour, minute and second:
                Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
                Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
                Serial.print(':');  
                if ( ((epoch % 3600) / 60) < 10 ) {
                  // In the first 10 minutes of each hour, we'll want a leading '0'
                  Serial.print('0');
                }
                Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
                Serial.print(':'); 
                if ( (epoch % 60) < 10 ) {
                  // In the first 10 seconds of each minute, we'll want a leading '0'
                  Serial.print('0');
                }
                Serial.println(epoch %60); // print the second




		//return epoch;
                return epoch-21600; //+3600; //+3600 -21600 for Central time +3600 for DST
	}else{
                Serial.println("UDP not available");
        }
	return 0;
}


// send an NTP request to the time server at the given address
void sendNTPpacket(byte* address){

	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);

	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision

	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
        // you can send a packet requesting a timestamp:         
        Udp.beginPacket(address, 123); //NTP requests are to port 123
        Udp.write(packetBuffer,NTP_PACKET_SIZE);
        Udp.endPacket(); 

}
