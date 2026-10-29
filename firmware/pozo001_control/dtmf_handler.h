#ifndef DTMF_HANDLER_H
#define DTMF_HANDLER_H

#include <Arduino.h>

class DTMFHandler {
public:
    DTMFHandler();
    void begin();
    void processTone(char tone);   // Procesa una tecla presionada en la llamada

    // Envío de tono de confirmación de audio al usuario durante la llamada
    void sendAudioFeedback(HardwareSerial& gsmSerial, char tone);

private:
    char lastTone;
    unsigned long lastToneTime;
};

extern DTMFHandler dtmfHandler;

#endif // DTMF_HANDLER_H
