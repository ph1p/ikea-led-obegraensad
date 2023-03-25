#include "mode/weather.h"
#include "secrets.h"

#ifdef ENABLE_SERVER

int lastUpdate = 0;

WiFiClient client;
HTTPClient http;

void weatherSetup()
{
    lastUpdate = millis();
    weatherUpdate();
}

void weatherLoop()
{
    if (lastUpdate >= millis()+1000 * 60 * 30){
        weatherUpdate();
        lastUpdate = millis();
    }
}

void weatherUpdate()
{
    String weatherApiString = String("http://api.openweathermap.org/data/2.5/weather?lat=")+LATITUDE+String("&lon=")+LONGITUDE+String("&units=metric&appid=")+WEATHERKEY;
    http.useHTTP10(true);
    http.begin(client,weatherApiString);
    http.GET();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc,http.getStream());

    int temperature = round(doc["main"]["temp"].as<float>());
    int conditions = doc["weather"][0]["id"].as<int>();
    int weatherIcon = 0;
    int iconY = 1;
    int tempY = 10;
    Serial.println(doc["weather"][0]["id"].as<int>());
    Serial.println(conditions);

    Screen.clear();

    if (200 <= conditions && conditions < 300){
        weatherIcon = 1;
    } else if (300 <= conditions && conditions < 400 || 500 <= conditions && conditions < 600){
        weatherIcon = 4;
    } else if (600 <= conditions && conditions < 700) {
        weatherIcon = 5;
    } else if (800 <= conditions && conditions < 900) {
        int cloudCover = (doc["clouds"]["all"].as<int>());
        if (cloudCover > 84){
            weatherIcon = 0;
            iconY = 2;
            tempY = 9;
        } else if (cloudCover < 11) {
            weatherIcon = 2;
            iconY = 2;
            tempY = 9;
        } else {
            weatherIcon = 3;
            iconY = 2;
        };
    } else if (700 <= conditions && conditions < 800) {
        weatherIcon = 6;
        iconY = 2;
    }

    Screen.drawWeather(0,iconY,weatherIcon);

    if (temperature >=10){
        Screen.drawCharacter(9,tempY,Screen.readBytes(degreeSymbol),4);
        Screen.drawNumbers(1, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
    } else if (temperature <=-10){
        Screen.drawCharacter(0,tempY,Screen.readBytes(minusSymbol),4);
        Screen.drawCharacter(11,tempY,Screen.readBytes(degreeSymbol),4);
        Screen.drawNumbers(3, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
    } else if (temperature >=0){
        Screen.drawCharacter(7,tempY,Screen.readBytes(degreeSymbol),4);
        Screen.drawNumbers(4, tempY, {temperature});
    } else {
        Screen.drawCharacter(0,tempY,Screen.readBytes(minusSymbol),4);
        Screen.drawCharacter(9,tempY,Screen.readBytes(degreeSymbol),4);
        Screen.drawNumbers(3, tempY, {temperature});
    }

    Screen.render();
}

#endif
