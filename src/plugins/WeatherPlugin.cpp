#include "plugins/WeatherPlugin.h"
#include "config.h"

// https://open-meteo.com/en/docs - no API key, WMO weather codes
#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
WiFiClient wiFiClient;
#endif

void WeatherPlugin::setup()
{
  Screen.clear();

#ifdef ESP32
  if (secureClient == nullptr)
  {
    secureClient = new WiFiClientSecure();
    secureClient->setInsecure();
  }
#endif

  // If we have cached data and it's still fresh (< 30 minutes old), redraw it
  if (hasCachedData && lastUpdate > 0 && millis() >= lastUpdate &&
      millis() - lastUpdate < (1000UL * 60 * 30))
  {
    Serial.println("Using cached weather data");
    drawWeather();
  }
  else
  {
    // Show loading screen - data needs to be fetched
    currentStatus = LOADING;
    Screen.setPixel(4, 7, 1);
    Screen.setPixel(5, 7, 1);
    Screen.setPixel(7, 7, 1);
    Screen.setPixel(8, 7, 1);
    Screen.setPixel(10, 7, 1);
    Screen.setPixel(11, 7, 1);
    currentStatus = NONE;

    // Clear lastUpdate to force immediate fetch on first loop
    this->lastUpdate = 0;
  }
}

void WeatherPlugin::loop()
{
  if (this->lastUpdate == 0 || millis() >= this->lastUpdate + this->updateInterval)
  {
    // come back in a few minutes after a failure instead of leaving the error
    // marker up for the whole refresh interval
    this->updateInterval = this->update() ? (1000UL * 60 * 30) : (1000UL * 60 * 3);
    this->lastUpdate = millis();
    Serial.println("updating weather");
  };
}

bool WeatherPlugin::update()
{
  // Check WiFi connection first
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected, skipping weather update");
    return false;
  }

  String weatherLocation = config.getWeatherLocation();
  Serial.print("[WeatherPlugin] Fetching weather for configured city: ");
  Serial.println(weatherLocation);

  // the forecast endpoint only accepts coordinates, so the city has to be geocoded
  // first - only repeat that when the configured city actually changed
  if (weatherLocation != resolvedLocation && !resolveLocation(weatherLocation))
  {
    drawError();
    return false;
  }
  resolvedLocation = weatherLocation;

  String weatherApiString = "https://api.open-meteo.com/v1/forecast?latitude=" +
                            String(latitude, 4) + "&longitude=" + String(longitude, 4) +
                            "&current=temperature_2m,weather_code";

  JsonDocument doc;
  if (!fetchJson(weatherApiString, doc))
  {
    drawError();
    return false;
  }

  int temperature = round(doc["current"]["temperature_2m"].as<float>());
  int weatherCode = doc["current"]["weather_code"].as<int>();
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
  else if (std::find(partyCloudyCodes.begin(), partyCloudyCodes.end(), weatherCode) !=
           partyCloudyCodes.end())
  {
    weatherIcon = 3;
    iconY = 2;
  }

  // Cache the weather data
  hasCachedData = true;
  cachedTemperature = temperature;
  cachedWeatherIcon = weatherIcon;
  cachedIconY = iconY;
  cachedTempY = tempY;

  // Draw the weather
  drawWeather();

  return true;
}

bool WeatherPlugin::resolveLocation(const String &location)
{
  String query = location;
  query.replace(" ", "+");

  JsonDocument doc;
  if (!fetchJson("https://geocoding-api.open-meteo.com/v1/search?name=" + query +
                     "&count=1&language=en&format=json",
                 doc))
  {
    return false;
  }

  if (!doc["results"][0]["latitude"].is<float>())
  {
    Serial.print("[WeatherPlugin] No coordinates found for ");
    Serial.println(location);
    return false;
  }

  latitude = doc["results"][0]["latitude"].as<float>();
  longitude = doc["results"][0]["longitude"].as<float>();

  Serial.print("[WeatherPlugin] Coordinates: ");
  Serial.print(latitude, 4);
  Serial.print(", ");
  Serial.println(longitude, 4);

  return true;
}

bool WeatherPlugin::fetchJson(const String &url, JsonDocument &doc)
{
#ifdef ESP32
  if (secureClient == nullptr)
  {
    Serial.println("Secure client not initialized!");
    return false;
  }
  http.begin(*secureClient, url);
#endif
#ifdef ESP8266
  http.begin(wiFiClient, url);
#endif

  http.setTimeout(20000);

  Serial.print("[WeatherPlugin] API request: ");
  Serial.println(url);

  int code = http.GET();
  Serial.print("HTTP response code: ");
  Serial.println(code);

  bool parsed = false;

  if (code == HTTP_CODE_OK)
  {
    // getString() decodes chunked transfer encoding, which the forecast endpoint
    // uses - parsing the raw stream would feed the chunk headers to the parser
    String payload = http.getString();
    DeserializationError error = deserializeJson(doc, payload);
    parsed = !error;

    if (error)
    {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
    }
  }
  else
  {
    Serial.print("HTTP request failed with code: ");
    Serial.println(code);
  }

  http.end();

  return parsed;
}

void WeatherPlugin::drawError()
{
  Screen.clear();

  Screen.setPixel(7, 4, 1);
  Screen.setPixel(8, 4, 1);
  Screen.setPixel(7, 5, 1);
  Screen.setPixel(8, 5, 1);
  Screen.setPixel(7, 6, 1);
  Screen.setPixel(8, 6, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(7, 8, 1);
  Screen.setPixel(8, 8, 1);

  Screen.setPixel(7, 10, 1);
  Screen.setPixel(8, 10, 1);
  Screen.setPixel(7, 11, 1);
  Screen.setPixel(8, 11, 1);
}

void WeatherPlugin::teardown()
{
  http.end();
}

void WeatherPlugin::drawWeather()
{
  Screen.clear();
  Screen.drawWeather(0, cachedIconY, cachedWeatherIcon, MAX_BRIGHTNESS);

  int temperature = cachedTemperature;
  int tempY = cachedTempY;

  if (temperature >= 10)
  {
    Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, MAX_BRIGHTNESS);
    Screen.drawNumbers(1, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
  }
  else if (temperature <= -10)
  {
    Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
    Screen.drawCharacter(11, tempY, Screen.readBytes(degreeSymbol), 4, MAX_BRIGHTNESS);
    temperature *= -1;
    Screen.drawNumbers(3, tempY, {(temperature - temperature % 10) / 10, temperature % 10});
  }
  else if (temperature >= 0)
  {
    Screen.drawCharacter(7, tempY, Screen.readBytes(degreeSymbol), 4, MAX_BRIGHTNESS);
    Screen.drawNumbers(4, tempY, {temperature});
  }
  else
  {
    Screen.drawCharacter(0, tempY, Screen.readBytes(minusSymbol), 4);
    Screen.drawCharacter(9, tempY, Screen.readBytes(degreeSymbol), 4, MAX_BRIGHTNESS);
    Screen.drawNumbers(3, tempY, {-temperature});
  }
}

const char *WeatherPlugin::getName() const
{
  return "Weather";
}
