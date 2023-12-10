#include "msp-controller.hpp"

void MSPController::clearDisplay() {
  this->msp.command(MSP_SET_NAME, const_cast<char*>("    "), 4, false);
}

void MSPController::setup(Stream & stream) {
  MSPController::msp.begin(stream);

  this->clearDisplay();

  logLine("MSP Controller setup complete");
}

void MSPController::printMessage(String message, unsigned long durationMs) {
  logLine("MSPController.printMessage called with");
  logInline("Message: ");
  logLine(message);

  if (message.length() > 16) {
    logLine("MSPController.printMessage: message too long, will be truncated in OSD...");
  }

  // add message to queue
  logLine("MSPController.printMessage: looking for empty queue slot");
  bool added = false;
  uint8_t indexToUse = 0;
  for (int i = 0; i < 10; i++) {
    if (this->messageQueue[i].active == false) {
      indexToUse = i;
      added = true;
      break;
    }
  }

  if (!added) {
    logLine("MSPController.printMessage: queue is full, clearing");
    for (int i = 0; i < 10; i++) {
      // delete all queue items
      this->messageQueue[i].active = false;
    }
    indexToUse = 0;
  }

  logLine("MSPController.printMessage: adding message to queue at index " + String(indexToUse));

  // copy string message so it doesn't get deleted
  this->messageQueue[indexToUse].durationMs = durationMs;
  this->messageQueue[indexToUse].message = String(message);
  this->messageQueue[indexToUse].active = true;

  if (this->nextMessageTime == 0) {
    logLine("MSPController.printMessage: queue is not running, enabling");

    this->nextMessageTime = millis();
  }
}

void MSPController::loop() {
  unsigned long now = millis();
  if (this->nextMessageTime == 0) {
    return;
  }

  if (this->nextMessageTime > now) {
    return;
  }

  // get next message from queue and shift queue
  msp_message message = this->messageQueue[0];

  if (message.active == false) {
    logLine("MSPController.Callback: no message in queue, clearing");
    this->clearDisplay();
    this->nextMessageTime = 0;
    return;
  }

  for (int i = 0; i < 9; i++) {
    this->messageQueue[i] = this->messageQueue[i + 1];
  }

  this->messageQueue[9].active = false;

  logInline("MSPController.Callback: printing message: ");
  this->msp.command(MSP_SET_NAME, (char*)message.message.c_str(), message.message.length(), false);
  logLine(message.message);

  this->nextMessageTime = now + message.durationMs;
}
