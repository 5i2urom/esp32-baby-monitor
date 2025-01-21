# Радионяня на ESP32

Этот проект представляет собой умную радионяню, построенную на базе микроконтроллера ESP32.  Устройство мониторит уровень шума, температуру и влажность в детской комнате и передает данные на мобильное устройство по Wi-Fi.  Конфигурация Wi-Fi и пороговое значение уровня шума задаются через Bluetooth.

## Возможности

* Мониторинг уровня шума, температуры и влажности.
* Передача данных на мобильное устройство по Wi-Fi.
* Настройка параметров Wi-Fi и порогового значения уровня шума по Bluetooth.
* Отображение данных на LCD дисплее.
* Веб-интерфейс для мониторинга данных.
* Предупреждение о превышении уровня шума (визуальное и на мобильном устройстве).

## Компоненты

* ESP32-WROOM32
* Датчик температуры и влажности DHT11
* Датчик звука KY-037
* LCD дисплей 2004 с I2C расширителем PCF8574

## Установка

1. Клонируйте репозиторий: `git clone https://github.com/5i2urom/esp32-baby-monitor`
2. Установите ESP-IDF
3. Настройте проект в ESP-IDF (укажите COM-порт и т.д.).
4. Скомпилируйте и загрузите прошивку на ESP32.

## Использование

1. Подключитесь к устройству по Bluetooth и настройте параметры Wi-Fi и порог шума.
2. После подключения к Wi-Fi, устройство отправит IP-адрес веб-сервера на мобильное устройство.
3. Откройте веб-интерфейс по полученному IP-адресу для мониторинга данных.

## Настройка

* Пороговое значение уровня шума можно настроить через Bluetooth.
* Параметры Wi-Fi (SSID и пароль) настраиваются через Bluetooth.
