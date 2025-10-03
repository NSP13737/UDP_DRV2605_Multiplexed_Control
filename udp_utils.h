#ifndef UDPMANAGER
#define UDPMANAGER

#include <WiFi.h>
#include <WiFiUdp.h>

void setupWireless(const char *ssid, const char *password, const int UdpPort);

std::array<float,8> getDistanceFloats(std::array<float,8> received_distances);

#endif