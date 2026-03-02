#include <Arduino.h>

// Định nghĩa các chân nhận tín hiệu từ RX MC6C
const uint8_t pwm_pins[] = {PA0, PA1, PA2, PA3, PA4, PA5};
volatile uint16_t pwm_values[6] = {1500, 1500, 1500, 1500, 1500, 1500};
volatile uint32_t rise_time[6] = {0};

// Các hàm ngắt đo xung
void isr0() { (digitalRead(PA0) == HIGH) ? rise_time[0] = micros() : pwm_values[0] = micros() - rise_time[0]; }
void isr1() { (digitalRead(PA1) == HIGH) ? rise_time[1] = micros() : pwm_values[1] = micros() - rise_time[1]; }
void isr2() { (digitalRead(PA2) == HIGH) ? rise_time[2] = micros() : pwm_values[2] = micros() - rise_time[2]; }
void isr3() { (digitalRead(PA3) == HIGH) ? rise_time[3] = micros() : pwm_values[3] = micros() - rise_time[3]; }
void isr4() { (digitalRead(PA4) == HIGH) ? rise_time[4] = micros() : pwm_values[4] = micros() - rise_time[4]; }
void isr5() { (digitalRead(PA5) == HIGH) ? rise_time[5] = micros() : pwm_values[5] = micros() - rise_time[5]; }

void setup() {
    // SBUS: 100000 baud, 8 data bits, Even parity, 2 stop bits
    Serial1.begin(100000, SERIAL_8E2);

    for(int i=0; i<6; i++) {
        pinMode(pwm_pins[i], INPUT_PULLUP);
    }

    attachInterrupt(digitalPinToInterrupt(PA0), isr0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA1), isr1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA2), isr2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA3), isr3, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA4), isr4, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PA5), isr5, CHANGE);
}

void loop() {
    static uint32_t last_send = 0;
    if (millis() - last_send >= 14) { // Gửi mỗi 14ms (Chuẩn SBUS)
        last_send = millis();

        uint16_t sbus_ch[16];
        for (int i = 0; i < 6; i++) {
            uint16_t p = constrain(pwm_values[i], 1000, 2000);
            sbus_ch[i] = map(p, 1000, 2000, 172, 1811);
        }
        for (int i = 6; i < 16; i++) sbus_ch[i] = 1024; // Các kênh còn lại để giữa

        uint8_t packet[25];
        packet[0] = 0x0F; // Start Byte
        packet[1] = (uint8_t) (sbus_ch[0] & 0x07FF);
        packet[2] = (uint8_t) ((sbus_ch[0] >> 8) | (sbus_ch[1] << 3));
        packet[3] = (uint8_t) ((sbus_ch[1] >> 5) | (sbus_ch[2] << 6));
        packet[4] = (uint8_t) (sbus_ch[2] >> 2);
        packet[5] = (uint8_t) ((sbus_ch[2] >> 10) | (sbus_ch[3] << 1));
        packet[6] = (uint8_t) ((sbus_ch[3] >> 7) | (sbus_ch[4] << 4));
        packet[7] = (uint8_t) ((sbus_ch[4] >> 4) | (sbus_ch[5] << 7));
        packet[8] = (uint8_t) (sbus_ch[5] >> 1);
        
        for(int i = 9; i < 23; i++) packet[i] = 0; // Tạm để trống
        packet[23] = 0x00; // Flags
        packet[24] = 0x00; // End Byte

        Serial1.write(packet, 25);
    }
}
