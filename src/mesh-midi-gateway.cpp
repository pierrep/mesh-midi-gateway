/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "/home/grimus/code/Particle/mesh-midi-gateway/src/mesh-midi-gateway.ino"
/* Mesh Sender */
int fetchMulticastAddress(IPAddress &mcastAddr);
void setup();
void loop();
void sendPacket(const char *msg);
#line 2 "/home/grimus/code/Particle/mesh-midi-gateway/src/mesh-midi-gateway.ino"
#define MAX_WIFI_PACKET_LEN (1232)
#define MAX_MESH_PACKET_LEN (128)
#define NUM_SLIDERS 5
#define NUM_TOGGLES 4

#include "OSCBundle.h"
#include "OSCMessage.h"

SYSTEM_THREAD(ENABLED);

static const uint16_t wifiPort = 8888;
static const uint16_t PORT = 31313;
static constexpr const char *MULTICAST_ADDR = "ff03::1:1001";

UDP udpWifi;
UDP udpMesh;

int fetchMulticastAddress(IPAddress &mcastAddr)
{
    HAL_IPAddress addr = {};
    addr.v = 6;
    inet_inet_pton(AF_INET6, MULTICAST_ADDR, addr.ipv6);
    mcastAddr = addr;
    return 0;
}

void setup()
{
    //WiFi.clearCredentials();
    waitUntil(WiFi.ready);

    Serial.begin();

    Serial.printlnf("Starting... This device ID = %s", System.deviceID().c_str());
    IPAddress myIP = WiFi.localIP();
    Serial.println(myIP); // prints the device's local IP address

    udpWifi.begin(wifiPort);
    udpWifi.setBuffer(MAX_WIFI_PACKET_LEN);
    Serial.printlnf("Listening for UDP packets on port %d", wifiPort);

    uint8_t idx = 0;
    if_name_to_index("th1", &idx);
    udpMesh.setBuffer(MAX_MESH_PACKET_LEN);
    udpMesh.begin(PORT, idx);
    Serial.printlnf("udp.begin() called on interface index = %d", idx);
    Serial.printlnf("my mesh IP = %s", Mesh.localIP().toString().c_str());
}

void loop()
{
    OSCMessage msg_received;
    char msg[MAX_MESH_PACKET_LEN];

    int bytesToRead = udpWifi.parsePacket();
    if (bytesToRead > 0)
    {
        while (bytesToRead--)
        {
            msg_received.fill(udpWifi.read());
        }
        if (!msg_received.hasError())
        {
            for (int i = 0; i < NUM_SLIDERS; i++)
            {
                String num = String(i + 1);
                String oscAddress;
                oscAddress = "/1/fader" + num;
                if (msg_received.fullMatch(oscAddress, 0))
                {
                    memset(msg, 0, strlen(msg)); //clear buffer
                    snprintf(msg, sizeof(msg), "f %i %f", i + 1, msg_received.getFloat(0));
                    sendPacket(msg);
                }
            }
            for (int i = 0; i < NUM_TOGGLES; i++)
            {
                String num = String(i + 1);
                String oscAddress;
                oscAddress = "/1/toggle" + num;
                if (msg_received.fullMatch(oscAddress, 0))
                {
                    memset(msg, 0, strlen(msg)); //clear buffer
                    snprintf(msg, sizeof(msg), "i %i %i", i + 1, (int)msg_received.getFloat(0));
                    sendPacket(msg);
                }
            }
            String oscAddress;
            oscAddress = "/noteOn";
            if (msg_received.fullMatch(oscAddress, 0))
            {
                // Serial.printf("channel: %i", msg_received.getInt(0));
                // Serial.printf(" note: %i", msg_received.getInt(1));
                // Serial.printlnf(" velocity: %i", msg_received.getInt(2));
                memset(msg, 0, strlen(msg)); //clear buffer
                snprintf(msg, sizeof(msg), "n %i %i %i",(int)msg_received.getInt(0),(int)msg_received.getInt(1),(int)msg_received.getInt(2));
                sendPacket(msg);
            }
            oscAddress = "/noteOff";
            if (msg_received.fullMatch(oscAddress, 0))
            {
                // Serial.printf("channel: %i", msg_received.getInt(0));
                // Serial.printf(" note: %i", msg_received.getInt(1));
                // Serial.printlnf(" velocity: %i", msg_received.getInt(2));
                memset(msg, 0, strlen(msg)); //clear buffer
                snprintf(msg, sizeof(msg), "o %i %i %i",(int)msg_received.getInt(0),(int)msg_received.getInt(1),(int)msg_received.getInt(2));
                sendPacket(msg);
            }            
        }
    }
}

void sendPacket(const char *msg)
{
    IPAddress mcastAddr;
    fetchMulticastAddress(mcastAddr);
    udpMesh.beginPacket(mcastAddr, PORT);
    udpMesh.write((const uint8_t *)msg, MAX_MESH_PACKET_LEN);
    udpMesh.endPacket();
}
