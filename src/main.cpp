#include <Arduino.h>
#include "esd.h"
#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid     = "HOSTEL-WiFi";
const char* password = "";
WiFiServer server(80);
static TaskHandle_t CompTask = NULL;
QueueHandle_t peak_queue = xQueueCreate(2, sizeof(int32_t));
static hw_timer_t *timer = NULL;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

// A-weighting curve from 31.5 Hz ... 8000 Hz
//INPUT BUFFER
//_____________
void IRAM_ATTR onTimer() 
{
  BaseType_t task_woken = pdFALSE;
  // Read from I2S into buffer
  size_t num_bytes_read;
  // int32_t samples[SAMPLES];
  esp_err_t err = i2s_read(I2S_PORT,
                             (void*) samples,
                             BLOCK_SIZE,
                             &num_bytes_read,
                             portMAX_DELAY);    // no timeout
    int samples_read = num_bytes_read / 8;
    vTaskNotifyGiveFromISR(CompTask, &task_woken);
    portYIELD_FROM_ISR();

}  
//______________

void wifiTask(void* pvParameters){
  for(;;){
        unsigned int peak  ;
        WiFiClient client = server.available();   // listen for incoming clients  
         client.println("HTTP/1.1 200 OK");
         client.println("Content-type:text/html");
         client.println();
            
         client.print("<h1>Sound Detection System</h1> <br>");
         client.println();
        xQueueReceive(peak_queue,&peak,portMAX_DELAY);

         //detecting frequencies around 1kHz
    if (detectFrequency(&bell, 15, peak, 45, 68, true))
    {
        Serial.println("Detected bell");
        sendAlarm(0, "home/alarm/doorbell", 2000);
         if (client) {                             // if you get a client,
            Serial.println("New Client.");           // print a message out the serial port
            client.print("<h3>Bell detected</h3><br>");
            client.println();
         }
   }

    //detecting frequencies around 3kHz
  if (detectFrequency(&fireAlarm, 15, peak, 120, 144, true))
    {
        Serial.println("Detected fire alarm");
        sendAlarm(1, "home/alarm/fire", 10000);
        if (client) {                             // if you get a client,
            Serial.println("New Client.");           // print a message out the serial port
            client.print("<h3>Fire alarm detected</h3> <br>");
            client.println();
          }
  //
    }
 }
}
///////////////////////
void processingTask(void* pvParameters) 
{
  while (1) 
  { 
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
     integerToFloat(samples, real, imag, SAMPLES);
    // apply flat top window, optimal for energy calculations
    fft.Windowing(FFT_WIN_TYP_FLT_TOP, FFT_FORWARD);
    fft.Compute(FFT_FORWARD);
    // calculate energy in each bin
    calculateEnergy(real, imag, SAMPLES);
    // sum up energy in bin for each octave
    sumEnergy(real, energy, 1, OCTAVES);
    // calculate loudness per octave + A weighted loudness
    float loudness = calculateLoudness(energy, aweighting, OCTAVES, 1.0);
    unsigned int peak = (int)floor(fft.MajorPeak());
    xQueueSend(peak_queue,&peak,0);
    calculateMetrics(loudness);
}
  }

void setup(void)
{
    Serial.begin(115200);
    Serial.println("Configuring I2S...");
    esp_err_t err;

    // The I2S config as per the example_______
    const i2s_config_t i2s_config = 
    {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),      // Receive, not transfer
        .sample_rate = 22627,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,   // although the SEL config should be left, it seems to transmit on right
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,       // Interrupt level 1
        .dma_buf_count = 8,     // number of buffers
        .dma_buf_len = BLOCK_SIZE,      // samples per buffer
        .use_apll = true
    };

    const i2s_pin_config_t pin_config = 
    {
        .bck_io_num = 33,       // BCKL
        .ws_io_num = 25,        // LRCL
        .data_out_num = -1,     // not used (only for speakers)
        .data_in_num = 32       // DOUT
    };
    // Configuring the I2S driver and pins.
    // This function must be called before any I2S driver read/write operations.
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed installing driver: %d\n", err);
        while (true);
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed setting pin: %d\n", err);
        while (true);
    }
    Serial.println("I2S driver installed.");
    //__________
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();

  Serial.println("HTTP server started");
    xTaskCreatePinnedToCore(processingTask,
                          "Compute FFT",
                          5000,
                          NULL,
                          1,
                          &CompTask,
                          0);
    xTaskCreatePinnedToCore(wifiTask,
                          "Wifi task",
                          5000,
                          NULL,
                          1,
                          NULL,
                          1);                      
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);

    timerAlarmWrite(timer, 0.06*1000000, true);  // Repeat the alarm (3rd parameter)
    timerAlarmEnable(timer);   // Start an alarm
   
}

void loop(void)
{
   if (micros() - last < 45200) {
      // send mqtt metrics every 10s while waiting and no trigger is detected
      if (millis() - ts >= 1000 && bell == 0 && fireAlarm == 0)
      {
          sendMetrics("home/noise", mn, mx, sum / cnt);
          cnt = 0;
          sum = 0;
          mn = 9999;
          mx = 0;
          ts = millis();
      }
      return;
    }
    last = micros();
}
