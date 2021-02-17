// References:
// orignial base code from https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
// painlessMesh basic sketch
// logical button code https://microcontrollerslab.com/push-button-esp32-gpio-digital-input/
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"

#include <base64.h> // ADDITIONAL - To convert image to Base64
#include "painlessMesh.h" // ADDITIONAL - For painlessMesh

// ADDITIONAL - Define painlessMesh SSID, password, port
#define   MESH_PREFIX     "mesh"
#define   MESH_PASSWORD   "meshpassword"
#define   MESH_PORT       5555

// ADDITIONAL - Define a scheduler
Scheduler userScheduler;
// ADDITIONAL - Define a mesh
painlessMesh  mesh;

// ADDITIONAL - For logical button, using GPIO 16 to detect high/low
const int DETECTPIN = 16;
// ADDITIONAL - Define checkGPIO function
void checkGPIO() ;
// ADDITIONAL - Define checkGPIO schedule
Task taskCheckGPIO( TASK_INTERVAL * 1 , TASK_FOREVER, &checkGPIO );

void checkGPIO() {
  int GPIO_16_state = digitalRead(DETECTPIN);
  if ( GPIO_16_state == LOW )
  {
    Serial.println("GPIO is low");
    camera_fb_t * fb = NULL;

    // Take Picture with Camera
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // ADDITIONAL - Convert to base64
    size_t size = fb->len;
    String image_base64 = base64::encode((uint8_t *) fb->buf, fb->len);
    Serial.println(image_base64);

    esp_camera_fb_return(fb);

    String msg = image_base64;
    //msg += mesh.getNodeId();
    mesh.sendBroadcast(msg);
    Serial.println("Base64 broadcasted");
    // Flash LED to tell user that a photo is taken and transmitted
    digitalWrite(4, HIGH);
    delay(500);
    digitalWrite(4, LOW);
  }
  if ( GPIO_16_state == HIGH )
  {
    Serial.println("GPIO is high");
  }
  taskCheckGPIO.setInterval(TASK_SECOND * 30 ); // This task will run every 30 sec
}

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ADDITIONAL - Needed for painlessMesh library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();

  // ADDITIONAL - Start painlessMesh
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  //mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | COMMUNICATION );
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // ADDITIONAL -  for Logical Button. This will declare pin 16 as digital input 
  pinMode(DETECTPIN, INPUT);
  // ADDITIONAL - for built-in LED
  pinMode(4, OUTPUT);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA; // reduced to QVGA due to base64 convertion size limit
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
    }
  // Add checkGPIO task
  userScheduler.addTask( taskCheckGPIO );
  taskCheckGPIO.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
