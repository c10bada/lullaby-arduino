#include <AudioZero.h>
#include <SD.h>
#include <SPI.h> //  need to include the SPI library

const int nextPin = 12;
const int playPin = 11;
const int prevPin = 10;
// AudioZero drives the built-in DAC on A0 only — connect amp/speaker chain to A0, not a random D# pin.
const int LED = 32;

const int numSongs = 2;

// variables will change:
int songIndex = 0;
String songList[numSongs];
String currentSong;
int lastNextState = HIGH;
int lastPlayState = HIGH;
int lastPrevState = HIGH;


void populateSongList() {
  /* 
  It's a bit ugly but we're going to iterate over the files twice,
  First to determine the size of the Array
  Second to store the filenames in the Array
  */

  Serial.println("Looking for songs on the SD card...");
  File root = SD.open("/");
  File entry =  root.openNextFile();
  String filename;
  boolean isSong;
  int count = 0;
  while (entry) {
    filename = String(entry.name());
    isSong = !entry.isDirectory() && !filename.startsWith("_") && (filename.endsWith(".WAV") || filename.endsWith(".wav"));
    if (isSong) {
     count++;
    }
    entry.close();
    entry = root.openNextFile();
  }
  
  // reset for second loop
  
  root = SD.open("/");
  entry = root.openNextFile();
  count = 0;  
  while (entry && (count < numSongs)) {
    filename = String(entry.name());
    isSong = !entry.isDirectory() && !filename.startsWith("_") && (filename.endsWith(".WAV") || filename.endsWith(".wav"));
    if (isSong) {
      Serial.println("Adding '" + filename + "'");
      songList[count] = filename;
      count++;
    }
    entry.close();
    entry = root.openNextFile();
  }
  Serial.println(String(count) + " total songs added...");
}  

void playSong() {
  currentSong = songList[songIndex];
  File currentSongFile = SD.open(currentSong);
  if (!currentSongFile) {
    Serial.println("error opening song '" + currentSong + "'");
    return;
  }

  Serial.println("Playing " + currentSong);
  AudioZero.play(currentSongFile);
  AudioZero.end();
  Serial.println("End of file. Thank you for listening!");
}

void setup() {
  pinMode(LED, OUTPUT);
    
  // INPUT_PULLUP will return 1/HIGH until the button is pressed
  pinMode(nextPin, INPUT_PULLUP);
  pinMode(playPin, INPUT_PULLUP);
  pinMode(prevPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial setup!");

  // setup SD-card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SDCARD_SS_PIN)) {
    Serial.println(" failed!");
    return;
  }
  Serial.println(" done.");

  populateSongList();

  // // hi-speed SPI transfers
  // SPI.setClockDivider(4);

  // 44.1 kHz stereo 8-bit WAV => 88200 bytes/s (one byte per timer tick)
  AudioZero.begin(2 * 44100);
}

void loop() {
  // Read the value of the input. It can either be 1 or 0
  int nextState = digitalRead(nextPin);
  int playState = digitalRead(playPin);
  int prevState = digitalRead(prevPin);

  /* NEXT BUTTON */
  if (lastNextState == HIGH && nextState == LOW) {
    // next btn pressed  
    digitalWrite(LED, HIGH);
    Serial.println("Next button clicked...");
    songIndex = (songIndex + 1) % numSongs;
    playSong();  
    lastNextState = nextState;
  } else if (lastNextState == LOW && nextState == HIGH) {
    // next btn released
    digitalWrite(LED, LOW);
    lastNextState = nextState;
  }
  
  /* PLAY BUTTON */
  else if (lastPlayState == HIGH && playState == LOW) {
    // play btn pressed
    digitalWrite(LED, HIGH);
    Serial.println("Play button clicked..."); 
    playSong();
    lastPlayState = playState;
  } else if (lastPlayState == LOW && playState == HIGH) {       
    // play btn released
    digitalWrite(LED, LOW);
    lastPlayState = playState;
  } 

  /* PREVIOUS BUTTON */
  else if (lastPrevState == HIGH && prevState == LOW) {
    // previous btn pressed
    digitalWrite(LED, HIGH); 
    Serial.println("Previous button clicked...");
    songIndex = (songIndex - 1) % numSongs;
    // ensure songIndex is always >= 0
    songIndex = songIndex >= 0 ? songIndex : 0;
    // Serial.println("songIndex " + String(songIndex));
    playSong();
    lastPrevState = prevState;
  } else if (lastPrevState == LOW && prevState == HIGH) {
    // play btn released
    digitalWrite(LED, LOW);
    lastPrevState = prevState;
  }
}

