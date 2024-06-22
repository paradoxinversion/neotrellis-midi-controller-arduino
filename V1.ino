/*
  Neon Performer
*/
#include "Adafruit_NeoTrellisM4.h"
#include "MIDIUSB.h"

#define MIDI_CHANNEL     0  // default channel # is 0
// Set the value of first note, C is a good choice. Lowest C is 0.
#define FIRST_MIDI_NOTE 24
#define MAX_OCTAVES 5

// The NeoTrellisM4 object is a keypad and neopixel strip subclass
// that does things like auto-update the NeoPixels and stuff!
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

boolean *lit_keys;

// Define our colors
const uint32_t col_white = trellis.Color(255, 255, 255);
const uint32_t col_black = trellis.Color(105, 105, 105);
const uint32_t col_red = trellis.Color(255, 0, 0);
const uint32_t col_green = trellis.Color(0, 255, 0);
const uint32_t col_blue = trellis.Color(0, 0, 255);

// Button Setup
// These buttons are playable midi notes
const int midi_keys[] = {0,1,2,3,4,5,8,9,10,11,12,13};

const int octave_mod_buttons[] ={6,14};
const int play_button = 22;
const int midi_key_count = 12;
int current_octave = 0;


void color_midi_keyboard() {
  // One octave of keys, where 1 = black
  int black_keys[] = {0,1,0,1,0,0,1,0,1,0,1,0};

  // Color all of our keys
  for (int i=0; i<midi_key_count; i++) {
    // If the current index is a black key, color it
    // Otherwise, make it white
    if (black_keys[i] == 1){
      trellis.setPixelColor(midi_keys[i], col_red);
    } else {
      trellis.setPixelColor(midi_keys[i], col_white);
    }
  }
}


int keyboard_button_to_note(int key_num){
  const int octave_mod = current_octave * 12;
  int val;
  Serial.println(val);
  if (key_num < 6){
    // Serial.print("Keypad key: "); Serial.println(key_num);
    val = FIRST_MIDI_NOTE+octave_mod+key_num;
    // Serial.print("MIDI key: "); Serial.println(val);
    return val;
  }

  if (key_num >= 8 && key_num < 14){
    // Serial.print("Keypad key: "); Serial.println(key_num);
    val = FIRST_MIDI_NOTE+octave_mod+key_num - 2; // We skip two buttons on the first row
    // Serial.print("MIDI key: "); Serial.println(val);
    return val;
  }
  return -1;
}

void color_octave_mod(){
  int color_intensities[] = {50, 100, 150, 200, 255};
  uint32_t c = trellis.Color(0,color_intensities[current_octave],0);
  trellis.setPixelColor(octave_mod_buttons[0], c);
  trellis.setPixelColor(octave_mod_buttons[1], c);
}

int key_to_octave_mod(int key_num){
  if (key_num == 6 && current_octave < MAX_OCTAVES){
    current_octave = current_octave + 1;
    Serial.print("Octave Up"); Serial.println(current_octave);
    return 1;
  }
  if (key_num == 14 && current_octave > 0){
    Serial.println("Octave Down ");
    current_octave = current_octave - 1;
    Serial.print("Octave Down"); Serial.println(current_octave);
    return -1;
  }
  return 0;
}

void key_to_play(int key_num){
  if (key_num == 22){
    trellis.controlChange(0, 1);
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("Performance Pad V1 Test");
    
  trellis.begin();
  trellis.setBrightness(50);

  Serial.println("Enabling MIDI on USB");
  trellis.enableUSBMIDI(true);
  trellis.setUSBMIDIchannel(MIDI_CHANNEL);
  // UART MIDI messages sent over the 4-pin STEMMA connector (3.3V logic)
  Serial.println("Enabling MIDI on UART");
  trellis.enableUARTMIDI(true);
  trellis.setUARTMIDIchannel(MIDI_CHANNEL);
  lit_keys = new boolean[trellis.num_keys()];
  
  for (int i=0; i<trellis.num_keys(); i++) {
    lit_keys[i] = false;
  }

  color_midi_keyboard();
  color_octave_mod();
}
  
void loop() {
  midiEventPacket_t rx; // get midi info
  // put your main code here, to run repeatedly:
  trellis.tick();
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      Serial.print("Received: ");
      Serial.print(rx.header);
      Serial.print("-");
      Serial.print(rx.byte1);
      Serial.print("-");
      Serial.print(rx.byte2);
      Serial.print("-");
      Serial.println(rx.byte3);
      // Serial.println(rx)
      
    }
  } while (rx.header != 0);
  while (trellis.available()){
    keypadEvent e = trellis.read();
    color_midi_keyboard();
    color_octave_mod();
    int key = e.bit.KEY;  // shorthand for what was pressed
    int note = keyboard_button_to_note(key);
    int octaveChange = 0;
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      trellis.setPixelColor(key,trellis.Color(0, 0, 0));
      // If it's the play button, start the scene
      key_to_play(key);
      if (note != -1){
        Serial.println(note);
        trellis.noteOn(note, 64);
      }
      octaveChange = key_to_octave_mod(key);
    }
    else if (e.bit.EVENT == KEY_JUST_RELEASED) {
      if(note != -1){
        trellis.noteOff(note, 64);
      }
    }
  }
  
  delay(10);
}
