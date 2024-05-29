#include "TGAM.h"

// Non-blocking function for parse response.
bool TGAM_NS::TGAM::read()
{
    loop();
    return _status == STATUS_OK;
}

// Blocking function for parse response. Default timeout is 1s.
bool TGAM_NS::TGAM::readUntil(uint16_t timeout)
{
    uint32_t start = millis();
    do
    {
        loop();
        if (_status == STATUS_OK) break;
    } while (millis() - start < timeout);

    return _status == STATUS_OK;
}

void TGAM_NS::TGAM::loop()
{
    _status = STATUS_WAITING;
    if (_stream->available())
    {
        uint8_t ch = _stream->read();

        if (_debug)
        {
            // _debug->print(_index);
            // _debug->print("\t");
            _debug->print(ch, HEX);
            _debug->print(" ");
        }

        if (_index < sizeof(_payload))
        {
             _payload[_index] = ch;
        }

        switch (_index)
        {
            case 0:
                if (ch != 0xAA)
                {
                    if(_debug)
                    {
                        _debug->println("Wrong packet header signature (0xAA)");
                    }
                    _index = 0;
                    return;
                }

                break;

            case 1:
                if (ch != 0xAA)
                {
                    if(_debug)
                    {
                        _debug->println("Wrong packet header signature (0xAA)");
                    }
                    _index = 0;
                    return;
                }

                break;


            case 2:
                _dataLen = ch;
                // Unsupported sensor, different frame length, transmission error e.t.c.
                if (_dataLen < 1 || _dataLen > sizeof(_payload))
                {
                    if(_debug)
                    {
                        _debug->print("Unsupported sensor, dataLen=");
                        _debug->println(_dataLen);
                    }
                    _index = 0;
                    return;
                }
                break;

            default:
                if (_index == PAYLOAD_OFFSET + _dataLen) // Checksum 
                {
                    _checksum = ch;
                    uint8_t _calculatedChecksum = calculateCheckSum(_payload, _dataLen);
                    if (_checksum != _calculatedChecksum)
                    {  
                        _index = 0;
                        if(_debug)
                        {
                            _debug->print("Wrong Checksum: ");
                            _debug->print(_checksum, HEX);
                            _debug->print(" Calculated: ");
                            _debug->println(_calculatedChecksum, HEX);
                        }
                        return;
                    }
                    if(_debug)
                    {
                        _debug->println();
                    }
                    _status = STATUS_OK;
                    _index = 0;
                    return;
                }
                else 
                {
                    _payload[_index - PAYLOAD_OFFSET] = ch;
                }

                break;
        }

        _index++;
    }
}

uint8_t TGAM_NS::TGAM::calculateCheckSum(uint8_t * payload, size_t len)
{
    uint16_t checksum = 0;
    for (int i = 0; i < len; i++)
    {
        // Serial.print("CS ");
        // Serial.print(i);
        // Serial.print("  ");
        // Serial.println(((const byte *)payload)[i]);
        checksum += ((const byte *)payload)[i];
    }

    return ~checksum & 0xFF;
}

void TGAM_NS::TGAM::dumpPayload()
{
    dump((const byte *)_payload, _dataLen);
}

void TGAM_NS::TGAM::dump(const unsigned char* data, const int len)
{
    if(_debug) 
    {
        for (int i=0; i<len; i++) 
        {
            char nibble, hexchar;

            nibble = (data[i] >> 4) & 0x0f;
            hexchar = nibble <= 9 ? '0' + nibble : 'A' + (nibble-10);
                                    
            _debug->print(hexchar);

            nibble = data[i] & 0x0f;
            hexchar = nibble <= 9 ? '0' + nibble : 'A' + (nibble-10);

            _debug->print(hexchar);
            _debug->print(' ');
        }
        _debug->println("");
    }
}

String TGAM_NS::TGAM::dumpPayloadToString()
{
    return dumpToString((const byte *)_payload, _dataLen);
}

String TGAM_NS::TGAM::dumpToString(const unsigned char* data, const int len)
{
    String result = "";

    for (int i=0; i<len; i++) 
    {
        char nibble, hexchar;

        nibble = (data[i] >> 4) & 0x0f;
        hexchar = nibble <= 9 ? '0' + nibble : 'A' + (nibble-10);

        result += hexchar;

        nibble = data[i] & 0x0f;
        hexchar = nibble <= 9 ? '0' + nibble : 'A' + (nibble-10);

        result += hexchar;
        result += ' ';
    }

    return result;
}

void TGAM_NS::TGAM::parsePayload()
{
    memset(values, 0, sizeof(values));

    for (size_t ptr = 0; ptr < _dataLen; ptr++) 
    {
        switch (_payload[ptr]) 
        {
            case 0x2:
                signalQuality = _payload[++ptr];
                break;
            case 0x4:
                att = _payload[++ptr];
                break;
            case 0x5:
                med = _payload[++ptr];
                break;
            case 0x83:
                ptr++;
                for (int j = 0; j < EEG_SIGNAL_BANDS; j++) 
                {
                	uint8_t a,b,c;
                    a = _payload[++ptr];
                    b = _payload[++ptr];
                    c = _payload[++ptr];
                    values[j] = ((uint32_t)a << 16) | ((uint32_t)b << 8) | (uint32_t)c;
                }
                break;
            case 0x80:
                // We dont' use this value so let's skip it and just increment i
                // uint8_t packetLength = packetData[++i];
                // rawValue = ((int)packetData[++i] << 8) | packetData[++i];
                ptr += 3;
                break;
            default:
                if (_debug)
                {
                    _debug->print("Wrong payload signature:");
                    _debug->println(_payload[ptr]);
                }
                break;
        }
    }

}

int TGAM_NS::TGAM::Setup(TGAM_NS::Config config, TGAM_NS::Baudrate initialBaudrate, TGAM_NS::Baudrate targetBaudrate)
{
    uint8_t mode2 = 0x63;
    _stream->write((uint8_t)mode2);
    _stream->flush();
    delay(2000);
    // _stream->end();
    // _stream->begin(57600);
    return 0;

    // Default TGAM 2.8 baudrate is 9600, start it
    _stream->begin((unsigned long)initialBaudrate);
    // while(!Serial) // Might be necessary for some arduino boards

    // Change baudrate
    // Page 6, baudrate selection
    uint8_t mode = 0x60 | (config.configMode == CONFIG_RAW ? 3 : config.configMode);
    _stream->write((uint8_t)mode);
    
    // HACK: We should now wait for a complete packet to be received,
    // as to verify everything is still working
    delay(2000);
    _stream->flush();
    _stream->begin((unsigned long)targetBaudrate);
    // while(!Serial) // Might be necessary for some arduino boards
    delay(5000);
    
    // First off, we must send the mode change command
    // 00000010 (0x02): 57.6k baud, normal+raw output mode
    _stream->write((uint8_t)config.configMode);
    delay(1000);
    
    // Config - Page 1
    // [0] RAW
    // [1] 10/8 bits (ENABLED)
    // [2] RAW Marker (DISABLED)
    mode = 0x12 | (config.configMode == CONFIG_RAW ? 0x01 : 0x00);
    _stream->write(mode);
    delay(1000);

    // Measuraments - Page 2
    // [1] Quality
    // [2] Power - int
    // [3] Power - float (DISABLED)
    // [4] Battery (DISABLED)
    mode = 0x20 | (config.quality ? 0x01 : 0x00) | (config.eegPowers ? 0x02 : 0x00);
    _stream->write(mode);
    delay(1000);

    // eSense - Page 3 (0011 << 8) | (0011)
    mode = 0x30 | (config.attention ? 0x01 : 0x00) | (config.meditation ? 0x02 : 0x00);
    _stream->write(mode);
    delay(1000);

    // Baud (page 6) will not be modified as it has already been done		
    return 0;
}