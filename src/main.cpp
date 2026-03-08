#include <Arduino.h>

#define CHANNELS 6

const uint8_t pwmPins[CHANNELS] = {PA0, PA1, PA2, PA3, PA4, PA5};

volatile uint16_t pwmRaw[CHANNELS] = {1500,1500,1500,1500,1500,1500};
volatile uint32_t riseTime[CHANNELS];

uint16_t sbusChannel[16];

void handleInterrupt(uint8_t ch)
{
    if (digitalRead(pwmPins[ch])) {
        riseTime[ch] = micros();
    } else {
        pwmRaw[ch] = micros() - riseTime[ch];
    }
}

void isr0(){handleInterrupt(0);}
void isr1(){handleInterrupt(1);}
void isr2(){handleInterrupt(2);}
void isr3(){handleInterrupt(3);}
void isr4(){handleInterrupt(4);}
void isr5(){handleInterrupt(5);}

void packSbus(uint8_t *packet)
{
    packet[0] = 0x0F;

    packet[1]  = (sbusChannel[0] & 0xFF);
    packet[2]  = ((sbusChannel[0] >> 8) | (sbusChannel[1] << 3)) & 0xFF;
    packet[3]  = ((sbusChannel[1] >> 5) | (sbusChannel[2] << 6)) & 0xFF;
    packet[4]  = (sbusChannel[2] >> 2) & 0xFF;
    packet[5]  = ((sbusChannel[2] >> 10) | (sbusChannel[3] << 1)) & 0xFF;
    packet[6]  = ((sbusChannel[3] >> 7) | (sbusChannel[4] << 4)) & 0xFF;
    packet[7]  = ((sbusChannel[4] >> 4) | (sbusChannel[5] << 7)) & 0xFF;
    packet[8]  = (sbusChannel[5] >> 1) & 0xFF;
    packet[9]  = ((sbusChannel[5] >> 9) | (sbusChannel[6] << 2)) & 0xFF;
    packet[10] = ((sbusChannel[6] >> 6) | (sbusChannel[7] << 5)) & 0xFF;
    packet[11] = (sbusChannel[7] >> 3) & 0xFF;
    packet[12] = (sbusChannel[8] & 0xFF);
    packet[13] = ((sbusChannel[8] >> 8) | (sbusChannel[9] << 3)) & 0xFF;
    packet[14] = ((sbusChannel[9] >> 5) | (sbusChannel[10] << 6)) & 0xFF;
    packet[15] = (sbusChannel[10] >> 2) & 0xFF;
    packet[16] = ((sbusChannel[10] >> 10) | (sbusChannel[11] << 1)) & 0xFF;
    packet[17] = ((sbusChannel[11] >> 7) | (sbusChannel[12] << 4)) & 0xFF;
    packet[18] = ((sbusChannel[12] >> 4) | (sbusChannel[13] << 7)) & 0xFF;
    packet[19] = (sbusChannel[13] >> 1) & 0xFF;
    packet[20] = ((sbusChannel[13] >> 9) | (sbusChannel[14] << 2)) & 0xFF;
    packet[21] = ((sbusChannel[14] >> 6) | (sbusChannel[15] << 5)) & 0xFF;
    packet[22] = (sbusChannel[15] >> 3) & 0xFF;

    packet[23] = 0x00; // flags
    packet[24] = 0x00; // end
}

void setup()
{
    Serial1.begin(100000, SERIAL_8E2); // SBUS

    for(int i=0;i<CHANNELS;i++)
        pinMode(pwmPins[i], INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PA0), isr0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA1), isr1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA2), isr2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA3), isr3, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA4), isr4, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA5), isr5, CHANGE);
}

void loop()
{
    static uint32_t lastSend = 0;

    if(millis() - lastSend >= 14)
    {
        lastSend = millis();

        for(int i=0;i<CHANNELS;i++)
        {
            uint16_t p = constrain(pwmRaw[i],1000,2000);
            sbusChannel[i] = map(p,1000,2000,172,1811);
        }

        for(int i=CHANNELS;i<16;i++)
            sbusChannel[i] = 1024;

        uint8_t packet[25];
        packSbus(packet);

        Serial1.write(packet,25);
    }
}
