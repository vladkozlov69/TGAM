#pragma once

#include "Arduino.h"

namespace TGAM_NS
{
    enum Baudrate
    {
        BAUD_SLOW       = 1200,
        BAUD_NORMAL     = 9600,
        BAUD_COMMANDS   = 38400,
        BAUD_FAST       = 57600
    };

    enum ConfigMode
    {
       CONFIG_DEFAULT  = 0x00, // Outputs normal data at BAUDRATE_NORMAL
       CONFIG_SLOW     = 0x01, // Outputs normal data at BAUDRATE_SLOW
       CONFIG_RAW      = 0x02, // Outputs normal and RAW data at BAUDRATE_FAST
       CONFIG_FFT      = 0x03, // Outputs FFT data at BAUDRATE_FAST
    };

    struct Config
    {
        ConfigMode configMode;
        bool quality;
        bool attention;
        bool meditation;
        bool eegPowers;

        Config(ConfigMode _configMode, bool _quality, bool _attention, bool _meditation, bool _eegPowers):
            configMode(_configMode),
            quality(_quality),
            attention(_attention),
            meditation(_meditation),
            eegPowers(_eegPowers)
        {
        }

        Config(ConfigMode configMode)
        {
            Config(configMode, true, true, true, true);
        }

        Config()
        {
            Config(CONFIG_DEFAULT, true, true, true, true);
        }
    };

    class TGAM
    {
    private:
        static const size_t MAX_DATA_SIZE = 128;
        static const size_t PAYLOAD_OFFSET = 3;
        static const size_t EEG_SIGNAL_BANDS = 8;
        enum STATUS { STATUS_WAITING, STATUS_OK };
        HardwareSerial * _stream;
        Print * _debug;
        STATUS _status;
        uint8_t _payload[MAX_DATA_SIZE];
        uint8_t _index = 0;
        uint8_t _dataLen = 0;
        uint8_t _checksum;
        uint8_t med, att, signalQuality;
        uint32_t values[EEG_SIGNAL_BANDS];
    private:
        void loop();  
        void dump(const unsigned char* data, const int len);  
        String dumpToString(const unsigned char* data, const int len);
    protected:
        uint8_t calculateCheckSum(uint8_t * payload, size_t len);
    public:
        enum EEG_SIGNAL_BAND {
            
        };
        static const uint16_t SINGLE_RESPONSE_TIME = 1000;
        TGAM(HardwareSerial * stream, Print * debug = NULL)
        {
            this->_stream = stream;
            this->_debug = debug;
        }
        bool read();
        bool readUntil(uint16_t timeout = SINGLE_RESPONSE_TIME);
        void dumpPayload();
        String dumpPayloadToString();
        int datalen()
        {
            return _dataLen;
        }
        short raw()
        {
            return (_payload[2]<<8) | _payload[3];
        }
        void parsePayload();
        uint8_t * payload()
        {
            return this->_payload;
        }
        int Setup(Config config, Baudrate initialBaudrate, Baudrate targetBaudrate);
        int Setup(ConfigMode configMode, Baudrate targetBaudrate)
        {
            return Setup(Config(configMode), BAUD_NORMAL, targetBaudrate);
        }
        inline int Setup(ConfigMode configMode)
        {
            switch (configMode)
            {
                case CONFIG_DEFAULT:
                    return Setup(configMode, BAUD_NORMAL);

                case CONFIG_SLOW:
                    return Setup(configMode, BAUD_SLOW);

                case CONFIG_RAW:
                case CONFIG_FFT:
                    return Setup(configMode, BAUD_FAST);
            }

            return -1;
        }

        uint8_t getAtt()
        {
            return att;
        }
        uint8_t getMed()
        {
            return med;
        }
        uint8_t getSignalQuality()
        {
            return signalQuality;
        }
        uint32_t getValue(int idx)
        {
            return values[idx];
        }
    };

}
