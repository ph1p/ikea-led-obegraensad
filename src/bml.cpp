// #include <stdlib.h>
// #include <Arduino.h>
// #include <tinyxml2.h>
// #include <led.h>

// using namespace tinyxml2;

// char const *document = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\"<blm width=\"26\" height=\"20\" bits=\"1\" channels=\"1\"><header><title>loop</title><description></description><creator>ArcadePaint</creator><author></author><email></email><loop>yes</loop><duration>1500</duration></header><frame duration=\"500\"><row>1111111111111111</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1111111111111111</row></frame><frame duration=\"1000\"><row>1111111111111111</row><row>1100000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1111111111111111</row></frame><frame duration=\"1500\"><row>1111111111111111</row><row>1110000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1111111111111111</row></frame><frame duration=\"1500\"><row>1111111111111111</row><row>1110000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000100000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1000000000000001</row><row>1111111111111111</row></frame></blm>";

// void setupXML()
// {
//   XMLDocument xmlDocument;

//   if (xmlDocument.Parse(document) != XML_SUCCESS)
//   {
//     Serial.println("Error parsing");
//     return;
//   };

//   XMLElement *root = xmlDocument.FirstChildElement();

//   uint8_t frameCount = 1;
//   for (XMLElement *frame = root->FirstChildElement("frame"); frame != NULL; frame = frame->NextSiblingElement("frame"))
//     frameCount++;

//   auto durationArray = new int[frameCount];
//   auto framesArray = new uint8_t[frameCount][256];

//   // frames
//   int arrayPosition = 0;
//   int currentFrame = 0;
//   for (XMLElement *frame = root->FirstChildElement("frame"); frame != NULL; frame = frame->NextSiblingElement("frame"))
//   {
//     durationArray[currentFrame] = frame->IntAttribute("duration");

//     for (XMLElement *row = frame->FirstChildElement("row"); row != NULL; row = row->NextSiblingElement("row"))
//     {
//       std::string my_string(row->GetText());

//       for (int i = 0; i < strlen(my_string.c_str()); i++)
//       {
//         framesArray[currentFrame][arrayPosition] = my_string[i] - '0';
//         arrayPosition++;
//       }
//     }
//     arrayPosition = 0;
//     currentFrame++;
//   }

//   int count = 0;
//   while (true)
//   {
//     if (count == frameCount-1)
//       count = 0;

//     renderScreen(framesArray[count]);
//     delay(durationArray[count]);
//     count++;
//   }
// }