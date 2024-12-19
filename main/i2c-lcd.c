#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "unistd.h"

void lcd_send_cmd(char cmd) {
    char data_u, data_l;
    uint8_t data_t[4];

    data_u = (cmd & 0xf0);
    data_l = ((cmd << 4) & 0xf0);

    data_t[0] = data_u | 0x0C;
    data_t[1] = data_u | 0x08;
    data_t[2] = data_l | 0x0C;
    data_t[3] = data_l | 0x08;

    i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
}

void lcd_send_data(char data) {
    char data_u, data_l;
    uint8_t data_t[4];

    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);

    data_t[0] = data_u | 0x0D;
    data_t[1] = data_u | 0x09;
    data_t[2] = data_l | 0x0D;
    data_t[3] = data_l | 0x09;

    i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    usleep(5000);
}

void lcd_set_cursor(int row, int col) {
    if (col < 0 || col >= 20) return;

    switch (row) {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
        case 2:
            col |= 0x94;
            break;
        case 3:
            col |= 0xD4;
            break;
    }
    lcd_send_cmd(col);
}

void lcd_init(void) {
    usleep(50000);
    lcd_send_cmd(0x30);
    usleep(5000);
    lcd_send_cmd(0x30);
    usleep(200);
    lcd_send_cmd(0x30);
    usleep(10000);
    lcd_send_cmd(0x20);
    usleep(10000);

    lcd_send_cmd(0x28);
    usleep(1000);
    lcd_send_cmd(0x08);
    usleep(1000);
    lcd_send_cmd(0x01);
    usleep(1000);
    lcd_send_cmd(0x06);
    usleep(1000);
    lcd_send_cmd(0x0C);
    usleep(1000);
}

void lcd_send_string(char *str) {
    while (*str) lcd_send_data(*str++);
}
