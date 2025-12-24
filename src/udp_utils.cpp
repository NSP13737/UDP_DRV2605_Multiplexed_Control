#include "udp_utils.h"
#include "debug.h"


namespace {
    WiFiUDP udp;
    uint8_t udp_recv_buffer[64]; //32 bytes to hold 8 distance floats + 8 study params
}

void udpSetupWireless(const char *ssid, const char *password, const int UdpPort) {
  
    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();
    debug("ESP32 AP IP: "); debugln(ip);

    // Start UDP server
    udp.begin(UdpPort);
    // debug("Listening on UDP port ");
    // debugln(localUdpPort);    
   
}

std::array<float,16> getData(std::array<float,16> current_data) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int len = udp.read(udp_recv_buffer, sizeof(udp_recv_buffer));
        if (len == static_cast<int>(sizeof(current_data))) { // make sure the information we received is the same size as what we are expecting
            memcpy(current_data.data(), udp_recv_buffer, sizeof(current_data));
            
        
        }
        
    }
    return current_data;
}