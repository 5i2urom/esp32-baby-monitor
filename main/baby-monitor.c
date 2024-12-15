#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c-lcd.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_22     
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      
#define I2C_MASTER_NUM              0                    
#define I2C_MASTER_FREQ_HZ          400000                    
#define I2C_MASTER_TX_BUF_DISABLE   0                         
#define I2C_MASTER_RX_BUF_DISABLE   0                          
#define I2C_MASTER_TIMEOUT_MS       1000

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void app_main(void)
{
    i2c_master_init();
    lcd_init();
    lcd_clear();

    char buffer[20];

    int temperature = 0; 
    int humidity = 60;     
    int noise_value = 50; 
    int noise_max_value = 100; 

    while (1) {
        sprintf(buffer, "Temp  : %d C", temperature);
        lcd_set_cursor(0, 0);
        lcd_send_string(buffer);

        sprintf(buffer, "Humi  : %d %%", humidity);
        lcd_set_cursor(1, 0);
        lcd_send_string(buffer);

        sprintf(buffer, "Noise : %d/%d", noise_value, noise_max_value);
        lcd_set_cursor(2, 0);
        lcd_send_string(buffer);

        // temperature = (temperature + 10) % 100;
        // humidity = (humidity + 10) % 100;
        // noise_value = (noise_value + 10) % 100;

        vTaskDelay(1000 / portTICK_PERIOD_MS); // задержка 1с

        lcd_clear();
    }

    
}
