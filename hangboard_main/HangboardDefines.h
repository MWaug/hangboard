#ifndef HangboardDefines
#define HangboardDefines

#define TEST_MODE true                 // Mock the hx711 sensor with fake data
#define HX711_DEFAULT_OFFSET 7785000   // hx711 output at zero lbs
#define HX711_DEFAULT_SCALE_UNIT 21100 // hx711 output per 1lb
#define HANG_THRESHOLD_DEFAULT_LB 20   // Force above this starts a hang
#define WEBSOCKET_SEND_INTERVAL_MS 50  // Minimum ms between websocket sends
#define WEBSOCKET_JSON_SIZE_BYTES 200
#define UART_FREQ_HZ 115200
#define MQTT_JSON_SIZE_BYTES 200

// Pin mappings
#define HX711_DOUT_PIN D2 // Node MCU v1.0 pin name
#define HX711_CLK_PIN D1  // Node MCU v1.0 pin name

#endif