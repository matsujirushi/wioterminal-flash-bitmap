#include <Arduino.h>
#include <ExtFlashLoader.h>
#include <SD/Seeed_SD.h>
#include <LovyanGFX.hpp>

///////////////////////////////////////////////////////////////////////////////
// setup and loop

static ExtFlashLoader::QSPIFlash Flash;
static auto FlashWallpaperAddress = reinterpret_cast<const uint8_t* const>(0x04200000);
static LGFX tft;

void setup()
{
    Serial.begin(115200);

    delay(1000);

    tft.begin();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextScroll(true);
    tft.setTextFont(2);

    Flash.initialize();    
	Flash.reset();
	Flash.enterToMemoryMode();

    Serial.print("SD.begin() ... ");
    if (SD.begin(SDCARD_SS_PIN, SDCARD_SPI))
    {
        Serial.print("SD.open() ... ");
        auto f = SD.open("IoTPnP.bmp", FILE_READ);
        if (!f)
        {
            Serial.println("ERROR");
            exit(1);
        }
        Serial.println("OK");

        auto size = f.size();
        Serial.print("Check file size ... ");
        if (size != 153604)
        {
            Serial.println("ERROR");
            exit(1);
        }
        Serial.println("OK");

        uint8_t buf[4096];

        // Skip header
        f.read(buf, 4);
        size -= 4;

        // Read bitmap data
        for (int i = 0; i < size; i += 4096)
        {
            const auto readSize = min(size - i, 4096);
            Serial.printf("Position=%d, Size=%d", i, readSize);

            Serial.print("  Read ... ");
            if (f.read(buf, readSize) != readSize)
            {
                Serial.println("ERROR");
                exit(1);
            }
            Serial.println("OK");

            Serial.print("  Write ... ");
            ExtFlashLoader::writeExternalFlash(Flash, (uintptr_t)(0x200000 + i), buf, readSize, [](std::size_t bytes_processed, std::size_t bytes_total, bool verifying) { return true; });
            Serial.println("OK");
        }
    }

//    tft.setSwapBytes(true);
    tft.pushImage(0, 0, 320, 240, (uint16_t*)FlashWallpaperAddress);
}

void loop()
{
}
