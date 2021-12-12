/*
    RF24Service.h - lib for nrf24 radio module based on RF24 v1.1.7 lib
    Created by Igor Tsiva 2021
    Released into the public domain.
*/

#ifndef RF24Service_h
#define RF24Service_h

#include "Arduino.h"
#include "nRF24L01.h"
#include "RF24.h"

class RF24Service: public RF24 {
    public:
        RF24Service(byte CE_PIN, byte CSE_PIN);

        void init(
            byte paLevel,
            byte dataRate,
            short delays,
            short retries
        );
        void init(byte paLevel, byte dataRate);
        void init();

        void withPayload(byte payloadSize);
        void withPayload();

        void asTransmitter(byte addressNo);
        void asTransmitter();

        void asReciever(byte pipeNo, byte addressNo);
        void asReciever(byte addressNo);
        void asReciever();

        void showDebug();
        void hideDebug();

        int scanChannels(byte numberOfScanRepeats);
        int scanChannels();
    private:
        byte _CE_PIN;
        byte _CSE_PIN;
        bool _isDebug = false;
        byte _address[6][6] = { "1Node", "2Node", "3Node", "4Node", "5Node", "6Node" };
};

#endif
