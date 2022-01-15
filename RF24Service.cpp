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
    stopListening();
    openWritingPipe(_address[addressNo]);
}

void RF24Service::asTransmitter() {
    stopListening();
    openWritingPipe(_address[0]);
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

// 63 - 90 is the best channel range
// int RF24Service::scanChannels(byte startChannel, byte endChannel, byte numberOfScanRepeats) {
//     _isScanning = true;

//     const byte numberOfScans = numberOfScanRepeats;
//     byte values[endChannel] = {0};
//     byte resultValues[endChannel] = {0};
//     unsigned short scanRepeats = 100;

//     if (_isDebug) {
//         Serial.println("Start Scanning for Free Channels...");
        
//         // Print out header, high then low digit
//         for (int i = startChannel; i < endChannel; i++) {
//             Serial.print(i>>4, HEX);
//         }
//         Serial.println();
//         for (int i = startChannel; i < endChannel; i++) {
//             Serial.print(i&0xf, HEX);
//         }
//         Serial.println();
//     }

//     for (int k = 0; k < numberOfScans; k ++) {
//         if (_isDebug) {
//             Serial.print("Scaning");
//         }

//         for (int i = 0; i < scanRepeats; i ++) {
//             if (_isDebug) {
//                 if (i == scanRepeats - 1) {
//                     Serial.print('/');
//                 } else {
//                     Serial.print('.');
//                 }
//             }

//             for (int j = startChannel; j < endChannel; j ++) {
//                 setChannel(j);
//                 startListening();
//                 delayMicroseconds(128);
//                 stopListening();
        
//                 if (testCarrier()){
//                     ++values[i];
//                 }
//             }
//         }
        
//         for (int i = startChannel; i < endChannel; i ++) {
//             resultValues[i] += values[i];
//             values[i] = 0;
//         }
        
//         if (_isDebug) {
//             Serial.println();
//         }
//     }

//     byte bestPositionStart = 0;
//     byte bestPositionClearLength = 0;
//     byte currentPositionStart = 0;
//     byte currentPositionClearLength = 0;
//     for (int i = startChannel; i < endChannel; i ++) {
//         // Ищим наиболее чистые участки эфира
//         if (!bestPositionClearLength && !resultValues[i]) {
//             bestPositionStart = i;
//             bestPositionClearLength ++;
//             currentPositionStart = i;
//             currentPositionClearLength ++;
//         } else if (!currentPositionClearLength && !resultValues[i]) {
//             currentPositionStart = i;
//             currentPositionClearLength ++;
//         } else if (currentPositionClearLength && !resultValues[i]) {
//             currentPositionClearLength ++;
//             if (i == endChannel - 1) {
//                 if (currentPositionClearLength > bestPositionClearLength) {
//                     bestPositionStart = currentPositionStart;
//                     bestPositionClearLength = currentPositionClearLength;
//                 }
//             }
//         } else if (currentPositionClearLength && resultValues[i]) {
//             if (currentPositionClearLength > bestPositionClearLength) {
//                 bestPositionStart = currentPositionStart;
//                 bestPositionClearLength = currentPositionClearLength;
//             }
//             currentPositionStart = 0;
//             currentPositionClearLength = 0;
//         }
//         if (_isDebug) {
//             Serial.print(resultValues[i], HEX);
//         }
//     }

//     if (_isDebug) {
//         Serial.println();
//     }

//     int resultBestStart = 0;
//     if (bestPositionClearLength > 5) {
//         resultBestStart = bestPositionStart + 2;
//     } else if (bestPositionClearLength > 3) {
//         resultBestStart = bestPositionStart + 1;
//     } else if (bestPositionClearLength) {
//         resultBestStart = bestPositionStart;
//     } else {
//         return -1;
//     }

//     _isScanning = false;

//     if (_isDebug) {
//         Serial.print("Best channel: ");
//         Serial.print(resultBestStart, HEX);
//         Serial.print(" ");
//         Serial.println(resultBestStart);
//     }
    
//     return resultBestStart;
// }

// int RF24Service::scanChannels(byte startChannel, byte endChannel) {
//     return scanChannels(startChannel, endChannel, 3);
// }

// int RF24Service::scanChannels(byte numberOfScanRepeats) {
//     return scanChannels(63, 90, numberOfScanRepeats);
// }

// int RF24Service::scanChannels() {
//     return scanChannels(63, 90);
// }

/**
 * @brief Searching for transmitter's channel, by specific key
 * transmitter have to send key if got key in ack payload
 * Have to be opened reading pipe before, after finish listening stops
 * Have to be enabled ack payload on both transmitter and reciever
 * transmitter will answer only if not connected yet
 */
// TODO implement read and write wrappers to add request time and
// time from last request

// TODO implement signal strength
// byte RF24Service::searchChannel (
//     byte key,
//     byte startChannel,
//     byte endChannel,
//     byte pipeId
// ) {
//     static bool isSetUp = false;
//     byte values[endChannel] = {0};
//     byte resultValues[endChannel] = {0};
//     unsigned short scanRepeats = 2;
//     _isScanning = true;

//     if (!isSetUp) {
//         Serial.println("Start Scanning for Transmitter...");
        
//         // Print out header, high then low digit
//         for (byte i = startChannel; i < endChannel; i++) {
//             Serial.print(i>>4);
//         }
//         Serial.println();
//         for (byte i = startChannel; i < endChannel; i++) {
//             Serial.print(i&0xf, HEX);
//         }
//         Serial.println();

//         isSetUp = true;
//     }

//     for (int i = 0; i < scanRepeats; i ++) {
//         Serial.println("");
//         for (byte j = startChannel; j < endChannel; j++) {
//             setChannel(j);
//             startListening();

//             delay(1000);

//             Serial.print(".");

//             byte pipeNo;
//             byte gotKey;
//             while (available(&pipeNo)) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
//                 read(&gotKey, sizeof(key));
//                 Serial.println("got data");
//                 Serial.print("Got key: ");
//                 Serial.println(gotKey);
//                 Serial.print("Key: ");
//                 Serial.println(key);

//                 if (pipeId && pipeId != pipeNo) {
//                     Serial.println("Wrong pipe");
//                     continue;
//                 }

//                 if (gotKey == key) {
//                     writeAckPayload(pipeNo, &key, sizeof(key));
//                     Serial.println("");
//                     Serial.print("FOUND: ");
//                     Serial.println(j, HEX);
//                     stopListening();
//                     _isScanning = false;
//                     return j;
//                 } else {
//                     continue;
//                 }
//             }

//             stopListening();
//         }
//     }

//     Serial.println();

//     _isScanning = false;
//     return -1;
// }

// byte RF24Service::searchChannel(byte key) {
//     byte startChannel = 63;
//     byte endChannel = 90;
//     byte pipeId = 1;
//     return searchChannel(key, startChannel, endChannel, pipeId);
// }

/**
 * @brief For transmitter
 * will send KEY in request
 * when reciever will send key back, it means connection established
 * isConnected() == true after it
 * 
 * @param key specific byte value for identification by reciever
 * @return isConnected()
 */
bool RF24Service::waitForConnection(byte key) {
    if (isConnected()) {
        return true;
    }

    bool isAnswered = write(&key, sizeof(key));

    if (isAnswered && available()) {
      byte resKey;
      read(&resKey, sizeof(key));

      if (resKey == key) {
        Serial.println("Connected!");
        connect();

        // Last time before connection false will be returned
        // to be able apply something right after connection
        return false;
      } else {
        Serial.print("Not OK Status!");
        Serial.println(resKey);
      }
    }

    return isConnected();
}

/**
 * @brief For reciever
 * will search for transmitter, that sent specific key
 * and connect to it
 * isConnected() == true after it
 * 
 * @param key specific byte value to find transmitter with same key
 * @return isConnected()
 */
// bool RF24Service::connectToSearchedChannel(byte key) {
//     if (isConnected()) {
//         return true;
//     }

    //byte channelFound = searchChannel(key);
    //Serial.println(channelFound);

    // if (channelFound != -1) {
    //   setChannel(channelFound);
    //   startListening();
    //   connect();

    //     // Last time before connection false will be returned
    //     // to be able apply something right after connection
    //   return false;
    // }
//     startListening();
//       connect();

//     return isConnected();
// }

// bool RF24Service::isScanning() {
//     return _isScanning;
// }

bool RF24Service::isError() {
    return _isError;
}

bool RF24Service::isConnected() {
    return _isConnected;
}

void RF24Service::disconnect() {
    _isConnected = false;
}

void RF24Service::connect() {
    _isConnected = true;
}

void RF24Service::resetFails() {
    _failCounter = 0;
}

void RF24Service::incrementFails() {
    _failCounter += 1;
}

byte RF24Service::fails() {
    return _failCounter;
}

bool RF24Service::send(const void* buf,	byte len) {
    unsigned long startTime = micros(); 
    bool isAnswered = write(buf, len);
    unsigned long endTime = micros();

    byte responseTimeLen = sizeof(_responseTime) / sizeof(_responseTime[0]);

    for (byte i = 0; i < responseTimeLen - 1; i++) {
        _responseTime[i] = _responseTime[i + 1];
    }

    _responseTime[responseTimeLen - 1] = endTime - startTime;

    if (isAnswered) {
        resetFails();
    } else {
        incrementFails();
    }

    return isAnswered;
}

// void RF24Service::get(void* buf, byte len) {
//     read(buf, len);
//     updateLastRequestTime();
// }

// void RF24Service::updateLastRequestTime() {
//     _lastRequestTime = micros();
// }

// unsigned long RF24Service::lastRequestTimeDiff() {
//     return micros() - _lastRequestTime;
// }

unsigned long RF24Service::lastResponseTime() {
    byte responseTimeLen = sizeof(_responseTime) / sizeof(_responseTime[0]);
    return _responseTime[responseTimeLen - 1];
}
