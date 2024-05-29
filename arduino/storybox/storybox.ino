/**
Story box

Code pour la boite à histoire, version arduino.

Librairies utilisées :
https://github.com/LennartHennigs/Button2 (latest) // simple button handler
https://github.com/pschatzmann/arduino-audio-tools #V0.9.7
https://github.com/pschatzmann/arduino-libhelix (latest)
https://github.com/jonnieZG/EWMA // smooth analog values (for smooth volume opperations)

à tester  : https://github.com/jonnieZG/EButton

 */

#include <SPI.h>
#include <SDFS.h>
// #include <SdFat.h>
#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include "Button2.h"
#include <Ewma.h>

/***************** CONFIG *******************/

// max number of stories in a directory
#define MAXSTORIES 50

// max volume levels for speaker and headphones
#define MAX_SPEAKER_VOLUME 1024  // 0 -> 1024 // boost > 1024
#define MAX_HEADPHONE_VOLUME 150 // 0 -> 1024

#define JACK_DETECTION_LEVEL 800 // bellow this value it will be considered speaker, above jack

#define VOLUME_STEPS 32 // How many steps there are in the volume control (max 1024)

#define DEBUG true

/****************** no more config below vvvv **************/

/********************** HARDWARE PINS *************************/
#define SDCARD_SPI SPI
#define SDCARD_CS_PIN 17

#define BUTTON_LEFT_PIN 10
#define BUTTON_RIGHT_PIN 11
#define BUTTON_PLAY_PIN 12
#define BUTTON_HOME_PIN 13

#define I2S_BCK 20
#define I2S_WS 21
#define I2S_DATA 22

#define BATTERY_ADC_PIN 26
#define VOLUME_PIN 27
#define JACK_PIN 28

#define BTN_DEBOUNCE_MS 50
#define COPIER_BUFFER_SIZE 512

#define HEADPHONE 0
#define SPEAKER 1

#define is_menu 0
#define is_play 1
#define is_pause 2

// #define SPI_CLOCK SD_SCK_MHZ(2)
// #define MP3_MAX_OUTPUT_SIZE 1024
// #define MP3_MAX_FRAME_SIZE 800

// #define MP3_MAX_OUTPUT_SIZE 1024 * 5 // 1024 * 5
// #define MP3_MAX_FRAME_SIZE 3200      // 1600
// #define I2S_BUFFER_SIZE 1024

/* end hardware config */

I2SStream i2s;
VolumeStream volumer(i2s);
EncodedAudioStream decoder(&volumer, new MP3DecoderHelix());
File audioFile;
// StreamCopy copier(decoder, audioFile, COPIER_BUFFER_SIZE);
StreamCopy copier;

Ewma smoother(0.01); // 0.1 : less smoothing // 0.01 : More smoothing - less prone to noise, but slower to detect changes

int status = is_menu;

Button2 button_home;
Button2 button_left;
Button2 button_right;
Button2 button_play;

String currentpath = "";

int currentstory = 1;
int totalstories = 0;

String stories[MAXSTORIES];

int volume = 0;
int lastVolume = 0;

int output = HEADPHONE;
int lastOutput = HEADPHONE;

bool led = true;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  blink(1);

  delay(100);
  Serial.println("*************** Story Box start V3 ****************");
  delay(100);

  // logger
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  // AudioLogger::instance().begin(Serial, AudioLogger::Info); // use this for more logs

  SDFSConfig sdconfig;
  sdconfig.setCSPin(SDCARD_CS_PIN);
  SDFS.setConfig(sdconfig);

  if (!SDFS.begin())
  {
    blink(5);
    Serial.printf("Init sd failed\n");
    // SDFS.errorHalt();
  }

  // i2s
  auto config = i2s.defaultConfig(TX_MODE);
  config.pin_bck = I2S_BCK;
  config.pin_ws = I2S_WS;
  config.pin_data = I2S_DATA;
  i2s.begin(config);

  // setup I2S based on sampling rate provided by decoder
  decoder.setNotifyAudioChange(i2s);
  decoder.begin();

  // copier buffer size
  copier.resize(COPIER_BUFFER_SIZE);

  // volume
  // auto volume_config = volumer.defaultConfig();
  // volume_config.allow_boost = true; // this allows volume > 1.0 be careful with this setting
  volumer.begin(config); // we need to provide the bits_per_sample and channels
  // volumer.begin(volume_config);
  volumer.setVolume(0.1);

  delay(100);

  // buttons
  button_left.begin(BUTTON_LEFT_PIN);
  button_left.setPressedHandler(handleTapLeft);

  button_right.begin(BUTTON_RIGHT_PIN);
  button_right.setPressedHandler(handleTapRight);

  button_home.begin(BUTTON_HOME_PIN);
  button_home.setPressedHandler(handleTapHome);

  button_play.begin(BUTTON_PLAY_PIN);
  button_play.setPressedHandler(handleTapPlay);

  handleDirectoryChange();
  play("/intro.mp3");
  digitalWrite(LED_BUILTIN, HIGH);
}

void blink(int times)
{
  delay(1000);
  for (int i = 0; i < times; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  delay(1000);
}

void play(String filename)
{
  audioFile = SDFS.open(filename, "r");
  copier.begin(decoder, audioFile);
  Serial.print("Play start ");
  Serial.println(filename);
}

void playmenu()
{
  status = is_menu;
  String menu = "/menu.mp3";
  play(currentpath + '/' + stories[currentstory] + menu);
}

void playstory()
{
  if (status == is_pause)
  {
    status = is_play;
    return;
  }
  status = is_play;
  play(currentpath + '/' + stories[currentstory] + "/story.mp3");
}

// currentpath
// currentstory = 1 .. int
// totalstories = int
// liste des dossier dans le dossier actuel -> tableau -> stories

void debug()
{
  if (DEBUG)
  {
    Serial.print("Current path : ");
    Serial.println(currentpath);
    Serial.print("totalstories : ");
    Serial.println(totalstories);
    Serial.print("Current story : ");
    Serial.println(currentstory);
    Serial.println("Stories found : ");
    for (int i = 0; i < totalstories; i++)
    {
      Serial.print("- ");
      Serial.println(stories[i]);
    }
    if (status == is_play)
    {
      Serial.println("*** playing ****");
    }

    if (status == is_menu)
    {
      Serial.println("*** on menu ****");
    }

    if (status == is_pause)
    {
      Serial.println("*** paused ****");
    }

    handleTemperature();
    handleBattery();
    handleJack();

    if (output == HEADPHONE)
    {
      Serial.println("Ouptut : headphone");
    }

    if (output == SPEAKER)
    {
      Serial.println("Ouptut : speaker");
    }

    Serial.print("Volume : ");
    Serial.println(volume);

    Serial.println("-----------------");
  }
}

void handleDirectoryChange()
{
  totalstories = 0;
  Dir dir = SDFS.openDir(currentpath);
  while (dir.next())
  {
    if (dir.isDirectory())
    {
      stories[totalstories] = dir.fileName();
      totalstories++;
    }
  }
}

void handleTapLeft(Button2 &b)
{
  Serial.println("button left pressed");
  currentstory--;
  if (currentstory < 0)
  {
    currentstory = totalstories - 1;
  }
  int i = 0;

  playmenu();
  debug();
}

void handleTapRight(Button2 &b)
{
  Serial.println("button right pressed");
  currentstory++;
  if (currentstory >= totalstories)
  {
    currentstory = 0;
  }
  playmenu();
  debug();
}

void handleTapHome(Button2 &b)
{
  Serial.println("button home pressed");
  currentpath = "";
  handleDirectoryChange();
  debug();
}

void handleTapPlay(Button2 &b)
{
  Serial.println("button play pressed");
  if (status == is_play)
  {
    status = is_pause;
  }
  else
  {
    playstory();
  }
  debug();
}

void handleBattery()
{
  float vsys = float(analogRead(29)) * 3.0 * 3.3 / 1024;
  Serial.print("vsys : ");
  Serial.println(vsys);

  float battery = float(analogRead(BATTERY_ADC_PIN)) * 2.0 * 3.3 / 1024 + 0.15; // the 0.15 is to acknowledge the voltage drop from the schotky diode
  Serial.print("battery : ");
  Serial.println(battery);
}

void handleTemperature()
{
  int temperature = analogReadTemp();
  Serial.print("Temperature : ");
  Serial.println(temperature);
}

/**
 * Reads the jack analog input 10 times, takes the highest signal,
 * if < JACK_DETECTION_LEVEL it means a jack is connected, reduce max volume for headphones
 * If > JACK_DETECTION_LEVEL it means a jack is not connected, we are in speaker mode, don't reduce volume
 *
 */
void handleJack()
{
  int max = 0;
  for (int i = 0; i < 10; i++)
  {
    int read = analogRead(JACK_PIN);
    if (read > max)
    {
      max = read;
    }
  }

  // Serial.print("Jack max : ");
  // Serial.println(max);

  if (max > JACK_DETECTION_LEVEL)
  {
    output = SPEAKER;
  }
  else
  {
    output = HEADPHONE;
  }
}

void handleVolume()
{
  volume = smoother.filter(analogRead(VOLUME_PIN));
  volume = map(volume, 0, 1024, 0, VOLUME_STEPS);

  if (volume != lastVolume || output != lastOutput)
  {
    lastVolume = volume;
    lastOutput = output;

    if (output == HEADPHONE)
    {
      volumer.setVolume(float(map(volume, 0, VOLUME_STEPS, 0, MAX_HEADPHONE_VOLUME)) / 1024);
    }

    if (output == SPEAKER)
    {
      volumer.setVolume(float(map(volume, 0, VOLUME_STEPS, 0, MAX_SPEAKER_VOLUME)) / 1024);
    }

    Serial.print("Volume : ");
    Serial.println(volume);
  }
}

void loop()
{

  led = !led;
  if (led)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  button_left.loop();
  button_right.loop();
  button_home.loop();
  button_play.loop();

  if (status == is_play || status == is_menu)
  {
    copier.copy();
  }

  handleVolume();
  handleJack();
}
