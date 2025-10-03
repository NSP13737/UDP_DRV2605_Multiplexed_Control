#include "udp_utils.h"


namespace {
    WiFiUDP udp;
    uint8_t udp_recv_buffer[32]; //32 bytes to hold 8 float values
}

void setupWireless(const char *ssid, const char *password, const int UdpPort) {
  
    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();
    Serial.print("ESP32 AP IP: ");
    Serial.println(ip);

    // Start UDP server
    udp.begin(UdpPort);
    // Serial.print("Listening on UDP port ");
    // Serial.println(localUdpPort);    
   
}

std::array<float,8> getDistanceFloats(std::array<float,8> received_distances) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int len = udp.read(udp_recv_buffer, sizeof(udp_recv_buffer));
        if (len == static_cast<int>(sizeof(received_distances))) { // make sure the information we received is the same size as what we are expecting
            memcpy(received_distances.data(), udp_recv_buffer, sizeof(received_distances));
            
        
        }
        
    }
    return received_distances;
}