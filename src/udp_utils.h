#ifndef UDPMANAGER
#define UDPMANAGER

#include <WiFi.h>
#include <WiFiUdp.h>

void udpSetupWireless(const char *ssid, const char *password, const int UdpPort);

/**
 * @brief Gets new data from UDP connection, if there is any.
 * @param current_data Contains 16 floats. The first 8 represent raw tactors distances, the next 8 represent study parameters.
 * @return If there is a new packet to recieve, returns floats for this, otherwise, returns values originally passed in.
 */
std::array<float,16> getData(std::array<float,16> current_data);

#endif