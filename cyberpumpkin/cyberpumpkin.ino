// Disclaimer
// This code was written almost exclusively at night in a hurry.
// It is not meant to be a demonstration of clean/performant/efficient code,
// if it works, it works.

// How does this work?
// By default, it shows an animation as stored in
// default_anim.h
// It connects to wifi, opens a TCP server on 8001
// and waits for connections.
// The body assumes the format of:
// first 3 bytes indicate mode, # frames, and speed
// and the rest 1024 byte sections are the frame data

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <ESP32Ping.h>
#include "wifi_ssid.h"

#define TIMEOUT 3000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define OLED_ADDRESS 0x3C

#define FRAME_SIZE 1024
#define ANIM_FRAMES 20
#define ANIM_BUFFER (FRAME_SIZE * ANIM_FRAMES)
#define ANIM_FRAME_TIME 100

#include "default_anim.h"

#define MODE_ANIMATE 1
#define MODE_PING 2
#define MODE_TEXT 3
#define MODE_NONE 4

// read from the buffer
byte mode = MODE_ANIMATE;
// time for each frame (multiply the byte read by 4)
int animFrameTime = DEFAULT_ANIM_TIME;
// number of frames
byte numframes = DEFAULT_ANIM_FRAMES;

int frame = 0;
ulong frametimer = 0;

unsigned char anim_buffer[ANIM_BUFFER] = DEFAULT_ANIM;

WiFiServer server(8001);

struct HttpRequest
{
    String request;
    char* body;
};

void setup()
{
    Serial.begin(115200);
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.display();

    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED)
    {
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(10, 10);
        display.println("Connecting...");
        display.display();
        Serial.println("connecting to wifi...");
        delay(500);
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("IP ");
    display.println(WiFi.localIP());
    display.display();

    delay(3500);
    server.begin();
}

void handleTCP(WiFiClient client)
{
    auto currentTime = millis();
    auto prevTime = currentTime;
    
    int cursor = 0;
    // todo timeouts
    while (client.connected())
    {
        // read first few bytes manually
        int numBytes;
        while(numBytes = client.available()) // && currentTime - prevTime < TIMEOUT
        {
            currentTime = millis();
            Serial.print("num bytes");
            Serial.println(numBytes);
            // on first loop, read first few bytes manually because they are special
            if (cursor == 0 && numBytes >= 3)
            {
                mode = client.read();
                animFrameTime = client.read() * 4;
                numframes = client.read();
                numBytes -= 3;

                Serial.print("mode is ");
                Serial.print(mode);
                Serial.print(" frame time ");
                Serial.print(animFrameTime);
                Serial.print(" num frames ");
                Serial.println(numframes);
            }
            // this can infinite loop
            // else if (cursor == 0 && numBytes != -1)
            // {
            //     Serial.println("Didn't read enough bytes, trying again");
            //     Serial.println(numBytes);
            //     continue;
            // }

            if (numBytes == -1)
            {
                Serial.println("num bytes was -1");
                delayMicroseconds(10);
                continue;
            }
            auto bytesRead = client.read((uint8_t*)anim_buffer + cursor, numBytes);

            if (bytesRead != numBytes)
            {
                Serial.print("read ");
                Serial.print(bytesRead);
                Serial.print(" but numBytes is ");
                Serial.println(numBytes);
            }

            cursor += numBytes;
            numBytes = client.available();
        }
    }
    client.stop();
}

void loop()
{
    server.setTimeout(TIMEOUT / 1000);
    WiFiClient client = server.available();

    if (mode == MODE_ANIMATE)
    {
        frame = ((millis() - frametimer) / animFrameTime) % numframes;
        display.clearDisplay();
        display.drawBitmap(0, 0, anim_buffer + FRAME_SIZE * frame, 128, 64, 1);
        display.display();
    }
    else if (mode == MODE_PING)
    {
        // use the anim buffer to store the host w/ a cstr
        String host((const char*)anim_buffer);
        pingTest(host);
        mode = MODE_NONE;
    }
    else if (mode == MODE_TEXT)
    {
        // use the anim buffer to store text
        String text((const char*)anim_buffer);
        handleTextMode(text);
        mode = MODE_NONE;
    }

    if (client)
    {
        display.clearDisplay();
        handleTCP(client);
    }
}

void pingTest(String body)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("PING: ");
    display.println(body);
    display.display();

    for (auto i = 0; i < 5; i++)
    {
        display.print("#");
        display.print(i);
        display.print(": ");
        display.display();
        auto result = Ping.ping(body.c_str());
        if (result)
        {
            display.print(Ping.averageTime());
            display.println(" ms");
        }
        else
        {
            display.println("ERR");
        }
        
        display.display();
        delay(200);
    }

    display.println("DONE");
    display.display();
}

void handleTextMode(String body)
{
    display.println(body);
    display.display();
}
