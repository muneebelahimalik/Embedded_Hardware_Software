
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "kiss_fft.h"


#define FFT_SIZE 2048
#define SAMPLE_RATE 48000
#define BIN_SIZE (SAMPLE_RATE / FFT_SIZE)
#define TARGET_FREQUENCY 1000
#define TARGET_BIN (TARGET_FREQUENCY / BIN_SIZE)

static char tag[] = "audio_task";

static void audio_task(void *pvParameters)
{
    i2s_config_t i2s_config = 
    {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .dma_buf_count = 3,
        .dma_buf_len = FFT_SIZE * 2,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .use_apll = 1
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, NULL);

    i2s_pin_config_t pin_config = 
    {
      .bck_io_num = 33,
      .ws_io_num = 25,
      .data_out_num = -1,
      .data_in_num = 32
    };
    i2s_set_pin(I2S_NUM_0, &pin_config);

    short sample_buf[FFT_SIZE];
    complex_t complex_buf[FFT_SIZE];

    while (1) {
        i2s_read(I2S_NUM_0, (char *)sample_buf, FFT_SIZE * 2, NULL, portMAX_DELAY);
        for (int i = 0; i < FFT_SIZE; i++) {
            complex_buf[i].real = sample_buf[i];
            complex_buf[i].imag = 0;
        }
        fft_transform(complex_buf, FFT_SIZE);
        if (complex_buf[TARGET_BIN].real > 1000) {
            ESP_LOGI(tag, "Alarm detected!");
        }
    }
  }

void app_main()
{
    xTaskCreate(audio_task, "audio_task", 8192, NULL, 5, NULL);
}