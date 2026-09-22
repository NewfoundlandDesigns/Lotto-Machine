Lotto Machine

A wireless, two-unit celebration trigger built on a pair of ESP32 boards. Press a button on the master unit and it plays an MP3, flashes its own LED strip, and — with no wires between them — tells a second, remote unit to flash its LED strip at the same time.

Built for the moment a winner gets picked and you want lights and sound to go off in two places at once.

How it works

Master (slotmachine.ino) — the button box:

A push button wired to the ESP32 triggers the sequence (guarded so a second press can't interrupt one already running).
It plays an MP3 track over a serial-connected player module.
It broadcasts a short message over Wi-Fi using ESP-NOW — no router, no pairing step, just a broadcast on a fixed channel.
It flashes its own NeoPixel LED strip green for 8 seconds.
The button's own indicator LED turns off for the duration of the sequence and back on when it's ready again.

Slave (ledslave/ledslave.ino) — the remote light unit:

Listens for ESP-NOW broadcasts and auto-registers whichever master sends one first — no manual pairing.
On receiving any message from a registered master, flashes its own NeoPixel strip green for 8 seconds.

The two units only need to share a Wi-Fi channel — there's no dependency on message content beyond "something happened," which keeps both sketches simple.

Hardware
Component	Notes
ESP32 dev board × 2	One per unit (master + slave)
WS2812/NeoPixel LED strip × 2	50 LEDs each (NUM_LEDS), one per unit
Push button with built-in LED	Master unit only
Serial MP3 player module (e.g. DFPlayer Mini) + speaker	Master unit only
Pin reference — master (slotmachine.ino)
Signal	GPIO
Button input	25
Button LED	33
LED strip data	18
MP3 module RX	16
MP3 module TX	17
Pin reference — slave (ledslave/ledslave.ino)
Signal	GPIO
LED strip data	33
Firmware
slotmachine.ino — flash to the master (button) unit.
ledslave/ledslave.ino — flash to the slave (remote light) unit.

Both are standard Arduino sketches for the ESP32 Arduino core.

Libraries required
Adafruit NeoPixel
ESP32 Arduino core, recent enough to include ESP32_NOW.h (the newer C++ ESP-NOW wrapper) and esp_mac.h
WiFi.h and SoftwareSerial (bundled with the ESP32 core)
Setup
Install the Arduino IDE and add ESP32 board support.
Install the Adafruit NeoPixel library.
Wire each unit per the pin tables above.
Flash slotmachine.ino to the master board and ledslave/ledslave.ino to the slave board.
Power both boards on — order doesn't matter, since the slave registers the first master it hears from.
Open the serial monitor at 115200 baud on either board to watch connection and trigger logs.
Press the button.

Both sketches are hardcoded to Wi-Fi channel 6 (ESPNOW_WIFI_CHANNEL) — change it in both files if that channel is in use nearby.

Known limitations
The MP3 track is hardcoded to track 1 (both at boot and on button press).
Flash color is hardcoded to green in both sketches.
ESP-NOW broadcasts are unencrypted and unaddressed by design here — any slave sketch on the same channel will react to any master's broadcast. Fine for a single-room build, not meant for a security-sensitive use.
No explicit button debounce beyond the isPlaying guard.
