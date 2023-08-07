#include "mode/weather.h"

#ifdef ENABLE_SERVER

unsigned long lastUpdate = 0;
const int httpTimeout = 10000;

HTTPClient http;

// https://github.com/chubin/wttr.in/blob/master/share/translations/en.txt
std::vector<int> thunderCodes = {200, 386, 389, 392, 395};
std::vector<int> cloudyCodes = {119, 122};
std::vector<int> partyCloudyCodes = {116};
std::vector<int> clearCodes = {113};
std::vector<int> fogCodes = {143, 248, 260};
std::vector<int> rainCodes = {
    176, 293, 296, 299, 302,
    305, 308, 311, 314, 353,
    356, 359, 386, 389, 263,
    266, 281, 284, 185};
std::vector<int> snowCodes = {
    179, 227, 323, 326, 329,
    332, 335, 338, 368, 371,
    392, 395, 230, 350};

void weatherSetup()
{
    lastUpdate = millis();
    weatherUpdate();
}

void weatherLoop()
{
    if (millis() >= lastUpdate + (1000 * 60 * 30))
    {
        weatherUpdate();
        lastUpdate = millis();
        Serial.println("updating weather");
    };
}

void weatherUpdate()
{
    String weatherApiString = "https://wttr.in/" + String(WEATHER_LOCATION) + "?format=j2&lang=en";
    http.begin(weatherApiString);
    http.setConnectTimeout(httpTimeout);
    http.setTimeout(httpTimeout);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK)
    {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, http.getString());

        int temperature = round(doc["current_condition"][0]["temp_C"].as<float>());
        int weatherCode = doc["current_condition"][0]["weatherCode"].as<int>();
        int weatherIcon = 0;
        int iconY = 1;
        int tempY = 10;

        if (std::find(thunderCodes.begin(), thunderCodes.end(), weatherCode) != thunderCodes.end())
        {
            weatherIcon = 1;
        }
        else if (std::find(rainCodes.begin(), rainCodes.end(), weatherCode) != rainCodes.end())
        {
            weatherIcon = 4;
        }
        else if (std::find(snowCodes.begin(), snowCodes.end(), weatherCode) != snowCodes.end())
        {
            weatherIcon = 5;
        }
        else if (std::find(fogCodes.begin(), fogCodes.end(), weatherCode) != fogCodes.end())
        {
            weatherIcon = 6;
            iconY = 2;
        }
        else if (std::find(clearCodes.begin(), clearCodes.end(), weatherCode) != clearCodes.end())
        {
            weatherIcon = 2;
            iconY = 1;
            tempY = 9;
        }
        else if (std::find(cloudyCodes.begin(), cloudyCodes.end(), weatherCode) != cloudyCodes.end())
        {
            weatherIcon = 0;
            iconY = 2;
            tempY = 9;
        }
        else if (std::find(partyCloudyCodes.begin(), partyCloudyCodes.end(), weatherCode) != partyCloudyCodes.end())
        {
            weatherIcon = 3;
            iconY = 2;
        }

        Screen.clear();
        Screen.drawWeather(0, iconY, weatherIcon, 100);

        if (temperature >= 10)
        {
            Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(1, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
        }
        else if (temperature <= -10)
        {
            Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
            Screen.drawCharacter(11, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(3, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
        }
        else if (temperature >= 0)
        {
            Screen.drawCharacter(7, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(4, tempY, {temperature});
        }
        else
        {
            Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
            Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, 50);
            Screen.drawNumbers(3, tempY, {temperature});
        }
    }
    else
    {
        Serial.print(F("HTTP request failed with code: "));
        Serial.println(httpResponseCode);
    }
}
#endif