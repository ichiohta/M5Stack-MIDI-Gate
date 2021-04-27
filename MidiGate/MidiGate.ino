/**
 * MIDI Pass Through Gate
 */

#include <M5Stack.h>
#include <MIDI.h>

// ---------------------------------------------------------------------- Constants

const int SIZE_INDICATOR =  20;
const int POS_X_VALUE    = 200;
const int POS_X_LABEL    =  10;
const int POS_Y_PASSTHRU =  10;
const int POS_Y_CC       =  50;
const int POS_Y_NOTE     = 130;
const int HEIGHT_LINE    =  30;

// ---------------------------------------------------------------------- Globals

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
bool passthru = 1;

// ---------------------------------------------------------------------- Visual Updaters

void updateIndicator(int x, int y, int sizeInPixels, bool enabled) {
  M5.Lcd.fillRect(x, y, sizeInPixels, sizeInPixels, enabled ? GREEN : RED);
}

void updateData(int x, int y, midi::DataByte data) {

  static const size_t DIGITS = 3;
  static char formatted[DIGITS + 1];

  sprintf(formatted, "%03d", data);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf(formatted);
}

void updatePassthru() {
  updateIndicator(POS_X_VALUE, POS_Y_PASSTHRU, SIZE_INDICATOR, passthru);
}

void updateMessageData(int y, midi::DataByte data1, midi::DataByte data2) {
  updateData(POS_X_VALUE, y,               data1);
  updateData(POS_X_VALUE, y + HEIGHT_LINE, data2);
}

void initializeDisplay() {

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(POS_X_LABEL, POS_Y_PASSTHRU);
  M5.Lcd.print("MIDI PassThru:");
  M5.Lcd.setCursor(POS_X_LABEL, POS_Y_CC);
  M5.Lcd.print("Control Number:");
  M5.Lcd.setCursor(POS_X_LABEL, POS_Y_CC + HEIGHT_LINE);
  M5.Lcd.print("Value:");
  M5.Lcd.setCursor(POS_X_LABEL, POS_Y_NOTE);
  M5.Lcd.print("Note Number:");
  M5.Lcd.setCursor(POS_X_LABEL, POS_Y_NOTE + HEIGHT_LINE);
  M5.Lcd.print("Velocity:");

  updateMessageData(POS_Y_CC, 0, 0);
  updateMessageData(POS_Y_NOTE, 0, 0);
  updatePassthru();
}

// ---------------------------------------------------------------------- Other Helpers

bool isPressed(Button button) {
  return button.wasReleased() || button.pressedFor(1000, 200);
}

// ---------------------------------------------------------------------- Entry Points

void setup() {

  M5.begin();
  MIDI.begin();
  MIDI.turnThruOff();

  initializeDisplay();
}

void loop() {

  M5.update();

  if (isPressed(M5.BtnA)) {
    passthru = !passthru;
    updatePassthru();
  }

  if (isPressed(M5.BtnB)) {
    for (int i = 0; i < 16; i++) {
      MIDI.send(midi::MidiType::ControlChange, midi::MidiControlChangeNumber::AllSoundOff, 0, i);
      MIDI.send(midi::MidiType::ControlChange, midi::MidiControlChangeNumber::AllNotesOff, 0, i);
    }
  }

  if (MIDI.read())
  {
    midi::MidiType type    = MIDI.getType();
    midi::Channel  channel = MIDI.getChannel();
    midi::DataByte data1   = MIDI.getData1();
    midi::DataByte data2   = MIDI.getData2();

    if (type == midi::ControlChange)
    {
      updateMessageData(POS_Y_CC, data1, data2);
    }
    else if (type == midi::NoteOn)
    {
      updateMessageData(POS_Y_NOTE, data1, data2);
    }

    if (passthru) {
      MIDI.send(type, data1, data2, channel);
    }
  }
}

// ---------------------------------------------------------------------- EOF
