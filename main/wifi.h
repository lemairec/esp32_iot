#ifndef WIFI_H_
#define WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int wifiInit();
extern void getUrl(char url_data[]);
extern void postRestWifi(char post_data[]);
extern void postRestConfigWifi(char post_data[]);

#ifdef __cplusplus
}
#endif

#endif