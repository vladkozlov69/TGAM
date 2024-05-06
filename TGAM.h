#pragma once

#include "Arduino.h"

class TGAM
{
private:
    static const size_t MAX_DATA_SIZE = 128;
    static const size_t PAYLOAD_OFFSET = 3;
    static const size_t EEG_SIGNAL_BANDS = 8;
    enum STATUS { STATUS_WAITING, STATUS_OK };
    Stream * _stream;
    Print * _debug;
    STATUS _status;
    uint8_t _payload[MAX_DATA_SIZE];
    uint8_t _index = 0;
    uint8_t _dataLen = 0;
    uint8_t _checksum;
    uint32_t values[EEG_SIGNAL_BANDS];
private:
    void loop();  
    void dump(const unsigned char* data, const int len);  
protected:
    uint8_t calculateCheckSum(uint8_t * payload, size_t len);
public:
    static const uint16_t SINGLE_RESPONSE_TIME = 1000;
    TGAM(Stream * stream, Print * debug = NULL)
    {
        this->_stream = stream;
        this->_debug = debug;
    }
    bool read();
    bool readUntil(uint16_t timeout = SINGLE_RESPONSE_TIME);
    void dumpPayload();
    void parsePayload();
};
