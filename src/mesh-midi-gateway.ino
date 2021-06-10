/* Mesh Sender */
#define MAX_WIFI_PACKET_LEN (1232)
#define MAX_MESH_PACKET_LEN (64)
#define NUM_SLIDERS 10
#define NUM_MODES 10

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
    Serial.printlnf("Waiting for WIFI to be read");
    //WiFi.clearCredentials();
    waitUntil(WiFi.ready);

    Serial.begin();

    Serial.printlnf("Starting... This device ID = %s", System.deviceID().c_str());
    IPAddress myIP = WiFi.localIP();
    Serial.printlnf("My WIFI localIP = %s",myIP.toString().c_str()); // prints the device's local IP address

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
                String num = String(i);
                String oscAddress;
                oscAddress = "/param" + num;
                if (msg_received.fullMatch(oscAddress, 0))
                {
                    memset(msg, 0, strlen(msg)); //clear buffer
                    snprintf(msg, sizeof(msg), "f %i %f", i, msg_received.getFloat(0));
                    sendPacket(msg);
                    //Serial.printf("parameter %i value: %f\n",i, msg_received.getFloat(0));
                }
            }
            String oscAddress;
            oscAddress = "/mode";
            if (msg_received.fullMatch(oscAddress, 0))
            {
                memset(msg, 0, strlen(msg)); //clear buffer
                uint8_t mode = msg_received.getInt(0);
                uint8_t value = msg_received.getInt(1);
                snprintf(msg, sizeof(msg), "m %i %i", mode, value);
                sendPacket(msg);
                //Serial.printf("mode change %i value: %i\n",mode, value);
            }               
            oscAddress = "/noteOn";
            if (msg_received.fullMatch(oscAddress, 0))
            {
                uint8_t bank = msg_received.getInt(0);
                uint8_t program = msg_received.getInt(1);
                uint8_t channel = msg_received.getInt(2);
                uint8_t note = msg_received.getInt(3);
                uint8_t velocity = msg_received.getInt(4);
                uint8_t duration = msg_received.getInt(5);
                // Serial.printf("bank: %i", bank);
                // Serial.printf(" program: %i", program);
                // Serial.printf(" channel: %i", channel);
                // Serial.printf(" note: %i", note);
                // Serial.printlnf(" velocity: %i", velocity);

                memset(msg, 0, strlen(msg)); //clear buffer
                
                snprintf(msg, sizeof(msg), "n %i %i %i %i %i %i",bank, program, channel, note, velocity, duration);
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
            oscAddress = "/meshConfig";
            if (msg_received.fullMatch(oscAddress, 0))
            {
                uint8_t id          = msg_received.getInt(0);
                uint8_t left        = msg_received.getInt(1);
                uint8_t top         = msg_received.getInt(2);
                uint8_t right       = msg_received.getInt(3);
                uint8_t bottom      = msg_received.getInt(4);
                uint8_t topleft     = msg_received.getInt(5);
                uint8_t topright    = msg_received.getInt(6);
                uint8_t bottomleft  = msg_received.getInt(7);
                uint8_t bottomright = msg_received.getInt(8);
                uint16_t totalDevices = msg_received.getInt(9);
                uint8_t gridWidth   = msg_received.getInt(10);
                uint8_t gridHeight  = msg_received.getInt(11);
                //  Serial.printf("/meshConfig \t\t device: %i", id);
                //  Serial.printf(" left: %i", left);
                //  Serial.printf(" top: %i", top);
                //  Serial.printf(" right: %i", right);
                //  Serial.printf(" bottom: %i", bottom);
                //  Serial.printf(" topleft: %i", topleft);
                //  Serial.printf(" topright: %i", topright);
                //  Serial.printf(" bottomleft: %i", bottomleft);
                //  Serial.printlnf(" bottomright: %i", bottomright);                 
                //  Serial.printlnf(" totalDevices: %i", totalDevices);  
                //  Serial.printlnf(" gridWidth: %i", gridWidth);  
                //  Serial.printlnf(" gridHeight: %i", gridHeight);  
                memset(msg, 0, strlen(msg)); //clear buffer
                snprintf(msg, sizeof(msg), "x %i %i %i %i %i %i %i",id,left,top,right,bottom,topleft,topright);
                //Serial.printlnf("meshConfig msg  = %s Size of packet = %i",msg,sizeof(msg));
                sendPacket(msg);
                memset(msg, 0, strlen(msg)); //clear buffer
                snprintf(msg, sizeof(msg), "y %i %i %i %i %i %i",id,bottomleft,bottomright,totalDevices, gridWidth, gridHeight);
                //Serial.printlnf("meshConfig msg  = %s Size of packet = %i",msg,sizeof(msg));
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
