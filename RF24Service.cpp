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

/**
 * @brief Wrapper over `write` method, with additional information i.e.
 * 
 * Last response time
 * Added response times in array to calculate signal quality
 * Count fails (no acknoledge for request)
 * 
 * @param buf 
 * @param len 
 * @return true 
 * @return false 
 */
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

/**
 * @brief Time in microsendos, that was taken for last request.
 * from start til end of `send` method
 * 
 * @return unsigned long 
 */
unsigned long RF24Service::lastResponseTime() {
    byte responseTimeLen = sizeof(_responseTime) / sizeof(_responseTime[0]);
    return _responseTime[responseTimeLen - 1];
}

/**
 * @brief Used on transmitter, if send method is used instead of write
 * 
 */
RF24Service::SIGNAL_QUALITY RF24Service::signalQuality() {
    byte responseTimeLen = sizeof(_responseTime) / sizeof(_responseTime[0]);
    unsigned int sum = 0;
    for (byte i = 0; i < responseTimeLen; i++) {
        sum += _responseTime[i];
    }

    unsigned int quality = sum / responseTimeLen;

    if (quality < 800) {
        return SIGNAL_QUALITY::PERFECT;
    } else if (quality < 1600) {
        return SIGNAL_QUALITY::GOOD;
    } else if (quality < 3200) {
        return SIGNAL_QUALITY::MODERATE;
    }

    return SIGNAL_QUALITY::BAD;
}
