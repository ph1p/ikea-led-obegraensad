#include "plugins/WeatherPlugin.h"

#ifdef ESP8266
WiFiClient wiFiClient;
#endif

void WeatherPlugin::setup()
{
    
    // loading screen
    Screen.clear();
    currentStatus = LOADING;
    Screen.setPixel(4, 7, 1);
    Screen.setPixel(5, 7, 1);
    Screen.setPixel(7, 7, 1);
    Screen.setPixel(8, 7, 1);
    Screen.setPixel(10, 7, 1);
    Screen.setPixel(11, 7, 1);

    this->lastUpdate = millis() - 1000 * 60 * WEATHER_UPDATE_FREQ + 15000;
    currentStatus = NONE;
}

void WeatherPlugin::loop()
{
    if (millis() >= this->lastUpdate + (1000 * 60 * WEATHER_UPDATE_FREQ))
    {

        if (!weatherSettingsLoaded)
        {
#ifdef ENABLE_STORAGE
            storage.begin("weather", false);
                if (this->weatherTime = storage.getInt("weatherTime",0))
                {
                    if (this->weatherMode = storage.getInt("weatherMode",0))
                    {
                        this->weatherSettingsLoaded = true;
                    }
                }
                
            storage.end();
#else
            this->weatherTime = 0;
            this->weatherMode = 0;
            this->weatherSettingsLoaded = true;
#endif
        }

        if (tick) //downloads and parses updates separately on alternating loops to avoid triggering CPU watchdog
        {
            this->getUpdate();
        } else {
            this->parseUpdate();
            this->lastUpdate = millis();
        }
    };
}

void WeatherPlugin::getUpdate()
{
    tryHtml:
    String weatherApiString;
    if (weatherMode == 0)
    {
      weatherApiString = String("https://api.open-meteo.com/v1/forecast?latitude=")+LATITUDE+String("&longitude=")+LONGITUDE+String("&current=temperature_2m,weather_code");
    } else if (weatherMode == 1) {
      weatherApiString = String("https://api.open-meteo.com/v1/forecast?latitude=")+LATITUDE+String("&longitude=")+LONGITUDE+String("&current=temperature_2m&minutely_15=temperature_2m,weather_code&timeformat=unixtime&forecast_days=2");
    } else {
        weatherApiString = String("https://api.open-meteo.com/v1/forecast?latitude=")+LATITUDE+String("&longitude=")+LONGITUDE+String("&current=temperature_2m&daily=temperature_2m_max,weather_code&timeformat=unixtime&forecast_days=7");
    }
#ifdef ESP32
    http.begin(weatherApiString);
#endif
#ifdef ESP8266
    http.begin(wiFiClient, weatherApiString);
#endif

    int code = http.GET();
    
    if (code == HTTP_CODE_OK)
    {
        tick = false;
    }
}

void WeatherPlugin::parseUpdate()
{
    DynamicJsonDocument doc(10144);
    deserializeJson(doc, http.getString());

    int currentTime = doc["current"]["time"].as<int>();
    int forecastPoint;

    if (weatherMode == 1)
    {
        //gets unix time forecast was provided, then does some math to look for closest 15-minute entry to the desired number of hours ahead
        forecastPoint = round((currentTime-(86400*floor(currentTime/86400)))/(15*60))+(weatherTime*4);
    } else if (weatherMode == 2) {
        forecastPoint = weatherTime;
    } else {
        forecastPoint = 0;
    }
    
    int temperature;
    int weatherCode;

    if (weatherMode == 0)
    {
        temperature = round(doc["current"]["temperature_2m"].as<float>());
        weatherCode = doc["current"]["weather_code"].as<int>();
    } else if (weatherMode == 1){
        temperature = round(doc["minutely_15"]["temperature_2m"][forecastPoint].as<float>());
        weatherCode = doc["minutely_15"]["weather_code"][forecastPoint].as<int>();
    } else {
        temperature = round(doc["daily"]["temperature_2m_max"][forecastPoint].as<float>());
        weatherCode = doc["daily"]["weather_code"][forecastPoint].as<int>();
    }
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
        iconY = 2;
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
    Screen.drawWeather(0, iconY, weatherIcon);

    if (temperature >= 10)
    {
        Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4);
        Screen.drawNumbers(1, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
    }
    else if (temperature <= -10)
    {
        Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
        Screen.drawCharacter(11, tempY, Screen.readBytes(degreeSymbol), 4);
        Screen.drawNumbers(3, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
    }
    else if (temperature >= 0)
    {
        Screen.drawCharacter(7, tempY, Screen.readBytes(degreeSymbol), 4);
        Screen.drawNumbers(4, tempY, {temperature});
    }
    else
    {
        Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
        Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4);
        Screen.drawNumbers(3, tempY, {temperature});
    }
    tick=true;
}

const char *WeatherPlugin::getName() const
{
    return "Weather";
}

void WeatherPlugin::websocketHook(DynamicJsonDocument &request)
{
  const char *event = request["event"];

  if (currentStatus == NONE)
  {
    if (!strcmp(event, "weatherTime"))
    {
      this->weatherTime = request["weatherTime"];
      this->lastUpdate = millis() - 1000 * 60 * WEATHER_UPDATE_FREQ; //this tricks the loop into forcing an update without messing up CPU pinning
#ifdef ENABLE_STORAGE
      storage.begin("weather", false);
        storage.putInt("weatherTime", this->weatherTime);
      storage.end();
#endif
    } else if (!strcmp(event, "weatherMode")) {
      this->weatherMode = request["weatherMode"];
      this->lastUpdate = millis() - 1000 * 60 * WEATHER_UPDATE_FREQ;
#ifdef ENABLE_STORAGE
      storage.begin("weather", false);
        storage.putInt("weatherMode", this->weatherMode);
      storage.end();
#endif
    }
  }
}

void WeatherPlugin::mqttHook(String topic, int message)
{
    if (topic == "Obegraensad/weatherMode")
    {
      this->weatherMode = message;
      this->lastUpdate = millis() - 1000 * 60 * WEATHER_UPDATE_FREQ;
    } else if (topic == "Obegraensad/weatherTime")
    {
      this->weatherTime = message;
      this->lastUpdate = millis() - 1000 * 60 * WEATHER_UPDATE_FREQ;
    }

}
