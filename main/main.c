#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "wifi.h"
#include "common/util.h"

char * version = "240701";

const char * company = "dizy";
const char * balise = "fioul";

int seconde_ev = 10;

int m_nb_minutes = 1;

void initIOT();
void onEventIOT();
void onEventConfig();

void onEventMinute(int minute){
    if(minute%m_nb_minutes == 0){
        onEventIOT();
    }
    if(minute%m_nb_minutes*100 == 0){
        onEventConfig();
    }
}

void onEventSecond(int second){
}



void vTaskWifi( void *pvParameters )
{
    wifiInit();
    int second = 0;
    for( ;; )
    {
        onEventSecond(second);
        if(second%60 == 0){
            onEventMinute(second/60);
        }
        second++;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void printInfos(){
    lc_DebugPrint("\n");
    lc_DebugPrint("\n");
    lc_DebugPrint("*** infos ");
    lc_DebugPrint("\n");
    lc_DebugPrint("Version : ");
    lc_DebugPrint(version);
    lc_DebugPrint("\n");
    lc_DebugPrint("company : ");
    lc_DebugPrint(company);
    lc_DebugPrint("\n");
    lc_DebugPrint("balise : ");
    lc_DebugPrint(balise);
    lc_DebugPrint("\n");
    char data[1024];
    snprintf(data, sizeof(data), "nb minutes : %i",m_nb_minutes);
    lc_DebugPrint(data);
    lc_DebugPrint("\n");
    lc_DebugPrint("wifi : ");
    lc_DebugPrint(getSsid());
    lc_DebugPrint("\n");
    lc_DebugPrint("pass : ");
    lc_DebugPrint(getPass());
    lc_DebugPrint("\n");
    
    lc_DebugPrint("*** fin infos ");
    lc_DebugPrint("\n");
    lc_DebugPrint("\n");
}


void app_main(void)
{
    printInfos();
    initIOT();
    xTaskCreate(
        vTaskWifi, /* Task function. */
        "vATaskFunction", /* name of task. */
        10000, /* Stack size of task */
        NULL, /* parameter of the task */
        1, /* priority of the task */
        NULL); /* Task handle to keep track of created task */

    while(true){
        vTaskDelay(10 / portTICK_RATE_MS);
       // verifyAlive();
    }
}


/***
 * IOT
*/




#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#include "i2cdev.h"
#include "vl53l1x.h"

static VL53L1_Dev_t dev;

#define TAG "VL53L1X"

static const I2cDef I2CConfig = {
    .i2cPort            = I2C_NUM_0,
    .i2cClockSpeed      = 400000,
    .gpioSCLPin         = 22,
    .gpioSDAPin         = 21,
    .gpioPullup         = GPIO_PULLUP_ENABLE,
};

I2cDrv i2cBus = {
    .def                = &I2CConfig,
};

VL53L1_Error status = VL53L1_ERROR_NONE;
VL53L1_RangingMeasurementData_t rangingData;
uint8_t dataReady = 0;
uint16_t range;


void initIOT(){
    if (vl53l1xInit(&dev, &i2cBus))
    {
        ESP_LOGI(TAG,"Lidar Sensor VL53L1X [OK]");
    }
    else
    {
        ESP_LOGI(TAG,"Lidar Sensor VL53L1X [FAIL]");
        return;
    }

    VL53L1_StopMeasurement(&dev);
    VL53L1_SetDistanceMode(&dev, VL53L1_DISTANCEMODE_MEDIUM);
    VL53L1_SetMeasurementTimingBudgetMicroSeconds(&dev, 25000);
}

void onEventDistance(){
    VL53L1_StartMeasurement(&dev);

    while (dataReady == 0)
    {
        status = VL53L1_GetMeasurementDataReady(&dev, &dataReady);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    status = VL53L1_GetRangingMeasurementData(&dev, &rangingData);
    range = rangingData.RangeMilliMeter;

    VL53L1_StopMeasurement(&dev);    

    VL53L1_StartMeasurement(&dev);

    ESP_LOGI(TAG,"Distance %d mm",range);

	char url[1024];
    double t1 = range/10.0;
	snprintf(url, sizeof(url), "https://www.maplaine.fr/silo/api_sonde?company=%s&balise=%s&t1=%f",company,balise,t1);
	getUrl(url);
}

void onEventIOT(){
    onEventDistance();
}

void onEventConfig(){
	snprintf(url, sizeof(url), "https://www.maplaine.fr/silo/api_sonde_config?company=%s&balise=%s&config=%s",company,balise,version);
	lc_DebugPrint(url);
	lc_DebugPrint("\n");
    //getUrl(url);
    lc_DebugPrint("\n");
}

