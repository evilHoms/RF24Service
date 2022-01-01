#include "Arduino.h"
#include "RF24Service.h"
#include "nRF24L01.h"
#include "RF24.h"

RF24Service::RF24Service(byte CE_PIN, byte CSE_PIN) : RF24(CE_PIN, CSE_PIN) {
    _CE_PIN = CE_PIN;
    _CSE_PIN = CSE_PIN;
}

void RF24Service::init(
    byte paLevel,
    rf24_datarate_e dataRate,
    byte payloadSize,
    short delays,
    short retries
) {
    while (!begin()) {
        if (_isDebug) {
            _isError = true;
            Serial.println(F("radio hardware is not responding!!"));
        }
        
        delay(1000);
    }
    _isError = false;

    setRetries(delays, retries);     //(время между попыткой достучаться, число попыток)
    setPayloadSize(payloadSize);     // максимальный размер пакета, в байтах
    
    setPALevel (paLevel); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    setDataRate (dataRate); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
    // ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!

    powerUp(); // усилить сигнал
}

void RF24Service::init(
    byte paLevel,
    rf24_datarate_e dataRate,
    byte payloadSize
) {
    init(paLevel, dataRate, payloadSize, 0, 15);
}

void RF24Service::init(
    byte paLevel,
    rf24_datarate_e dataRate
) {
    init(paLevel, dataRate, 32);
}

void RF24Service::init() {
    init(RF24_PA_MAX, RF24_1MBPS, 32);
}

void RF24Service::asTransmitter(byte addressNo) {
    openWritingPipe(_address[addressNo]);
    stopListening();
}

void RF24Service::asTransmitter() {
    openWritingPipe(_address[0]);
    stopListening();
}

void RF24Service::asReciever(byte id, byte addressNo) {
    openReadingPipe(id, _address[addressNo]);
    startListening();
}

void RF24Service::asReciever(byte addressNo) {
    openReadingPipe(1, _address[addressNo]);
    startListening();
}

void RF24Service::asReciever() {
    openReadingPipe(1, _address[0]);
    startListening();
}

void RF24Service::showDebug() {
    _isDebug = true;
}

void RF24Service::hideDebug() {
    _isDebug = false;
}

int RF24Service::scanChannels(byte numberOfScanRepeats) {
    _isScanning = true;

    const byte numChannels = 126;
    const byte numberOfScans = numberOfScanRepeats;
    byte values[numChannels] = {0};
    byte resultValues[numChannels] = {0};
    unsigned short scanRepeats = 100;

    if (_isDebug) {
        Serial.println("Start Scanning for Free Channels...");
        
        // Print out header, high then low digit
        for (int i = 0; i < numChannels; i++) {
            Serial.print(i>>4, HEX);
        }
        Serial.println();
        for (int i = 0; i < numChannels; i++) {
            Serial.print(i&0xf, HEX);
        }
        Serial.println();
    }

    for (int k = 0; k < numberOfScans; k ++) {
        if (_isDebug) {
            Serial.print("Scaning");
        }

        for (int i = 0; i < scanRepeats; i ++) {
            if (_isDebug) {
                Serial.print('.');
            }

            for (int j = 0; j < numChannels; j ++) {
                setChannel(j);
                startListening();
                delayMicroseconds(128);
                stopListening();
        
                if (testCarrier()){
                    ++values[i];
                }
            }
        }
        
        for (int i = 0; i < numChannels; i ++) {
            resultValues[i] += values[i];
            values[i] = 0;
        }
        
        if (_isDebug) {
            Serial.println();
        }
    }

    byte bestPositionStart = 0;
    byte bestPositionClearLength = 0;
    byte currentPositionStart = 0;
    byte currentPositionClearLength = 0;
    for (int i = 0; i < numChannels; i ++) {
        // Ищим наиболее чистые участки эфира
        if (!bestPositionClearLength && !resultValues[i]) {
            bestPositionStart = i;
            bestPositionClearLength ++;
            currentPositionStart = i;
            currentPositionClearLength ++;
        } else if (!currentPositionClearLength && !resultValues[i]) {
            currentPositionStart = i;
            currentPositionClearLength ++;
        } else if (currentPositionClearLength && !resultValues[i]) {
            currentPositionClearLength ++;
            if (i == numChannels - 1) {
                if (currentPositionClearLength > bestPositionClearLength) {
                    bestPositionStart = currentPositionStart;
                    bestPositionClearLength = currentPositionClearLength;
                }
            }
        } else if (currentPositionClearLength && resultValues[i]) {
            if (currentPositionClearLength > bestPositionClearLength) {
                bestPositionStart = currentPositionStart;
                bestPositionClearLength = currentPositionClearLength;
            }
            currentPositionStart = 0;
            currentPositionClearLength = 0;
        }
        if (_isDebug) {
            Serial.print(resultValues[i], HEX);
        }
    }

    if (_isDebug) {
        Serial.println();
    }

    int resultBestStart = 0;
    if (bestPositionClearLength > 5) {
        resultBestStart = bestPositionStart + 2;
    } else if (bestPositionClearLength > 3) {
        resultBestStart = bestPositionStart + 1;
    } else if (bestPositionClearLength) {
        resultBestStart = bestPositionStart;
    } else {
        return -1;
    }

    _isScanning = false;

    if (_isDebug) {
        Serial.print("Best channel: ");
        Serial.println(resultBestStart);
    }
    
    return resultBestStart;
}

int RF24Service::scanChannels() {
    return scanChannels(3);
}

bool RF24Service::isScanning() {
    return _isScanning;
}

bool RF24Service::isError() {
    return _isError;
}
