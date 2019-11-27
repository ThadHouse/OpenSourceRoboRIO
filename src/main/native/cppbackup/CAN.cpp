#include "hal/CAN.h"

#include "can/niEmbCAN.h"

#include <thread>
#include <atomic>
#include "wpi/SafeThread.h"
#include "wpi/DenseMap.h"

static tNiEmbCANHandle canHandle;

struct CANMessage {
  uint8_t data[8];
  int8_t length;
};



static void AddOrUpdateMessage(const tNiEmbCANFrame* frame) {
  uint32_t id = frame->m_identifier;
  if (!frame->m_isExtended) id |= HAL_CAN_IS_FRAME_11BIT;
  if (frame->m_isRemote) id |= HAL_CAN_IS_FRAME_REMOTE;

  if (!receivedMessages) {
    // Empty array, special case.
    receivedMessages = malloc(sizeof(struct CANMessage) * 4);
    messageCapacity = 4;
    messageCount = 1;
    receivedMessages[0].id = id;
    receivedMessages[0].length = frame->m_dataLength;
    memcpy(receivedMessages[0].data, frame->m_payload, 8);
    return;
  }

  // Start checking at half index
  int checkIndex = messageCount / 2;
  for(;;) {

  }
}




void* readThreadFunc(void* arg) {
  tNiEmbCANFrame frame;
  tNiEmbCANTimestamp timestamp;

  while (runThread) {
    tNiEmbCANStatus status = niEmbCANRead(canHandle, 250000, &frame, &timestamp);
    if (status != 0) continue;

  }
  return NULL;
}

extern "C" {

void HAL_CAN_Initialize(int32_t* status) {
  *status = niEmbCANOpenSession(0, 1000000, 32, &canHandle);

  niEmbCANStart(canHandle);

  runThread = 1;

  messageCount = 0;
  receivedMessages = NULL;

  pthread_create(&readThread, NULL, readThreadFunc, NULL);
}


}
