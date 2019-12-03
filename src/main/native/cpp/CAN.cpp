#include "hal/CAN.h"

#include "niEmbCAN.h"

#include <thread>
#include <atomic>
#include "wpi/SafeThread.h"
#include <vector>
#include "HALInitializer.h"
#include <algorithm>

static tNiEmbCANHandle canHandle;

namespace {
struct CANMessage {
  uint32_t messageID;
  uint64_t timeStamp;
  uint8_t data[8];
  int8_t dataSize;
};
}

static std::mutex canSendMutex;
static std::mutex canReceiveMutex;
static std::vector<CANMessage> canReceivedMessages;
static std::atomic_bool runThreads{false};
static std::thread periodicThread;
static std::thread receiveThread;

static void periodicThreadMain() {
  while (runThreads) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

static void receiveThreadMain() {
  tNiEmbCANFrame frame;
  tNiEmbCANTimestamp timestamp;


  while (runThreads) {
    tNiEmbCANStatus status = niEmbCANRead(canHandle, 250000, &frame, &timestamp);
    if (status != 0) continue;
    uint32_t frameId = frame.m_identifier;
    frameId |= static_cast<uint32_t>(!frame.m_isExtended) << 30;
    frameId |= static_cast<uint32_t>(frame.m_isRemote) << 31;

    std::scoped_lock lock(canReceiveMutex);
    auto found = std::find_if(canReceivedMessages.begin(), canReceivedMessages.end(), [&](const CANMessage& msg){
      return msg.messageID == frameId;
    });

    if (found == canReceivedMessages.end()) {
      // Not found
      CANMessage newMessage;
      newMessage.dataSize = frame.m_dataLength;
      newMessage.messageID = frameId;
      newMessage.timeStamp = timestamp;
      std::copy(std::begin(frame.m_payload), std::end(frame.m_payload), std::begin(newMessage.data));
      canReceivedMessages.emplace_back(newMessage);
    } else {
      found->dataSize = frame.m_dataLength;
      found->timeStamp = timestamp;
      std::copy(std::begin(frame.m_payload), std::end(frame.m_payload), std::begin(found->data));
    }
  }
}

namespace hal {
namespace init {
  void InitializeCAN() {
    int32_t status = 0;

    status = niEmbCANOpenSession(0, 1000000, 32, &canHandle);

    if (status != 0) {
      return;
    }

    status = niEmbCANStart(canHandle);

    if (status != 0) {
      niEmbCANCloseSession(canHandle);
      return;
    }

    std::atexit([]{
      runThreads = false;
      if (periodicThread.joinable()) periodicThread.join();
      if (receiveThread.joinable()) receiveThread.join();
      niEmbCANCloseSession(canHandle);
    });

    periodicThread = std::thread(periodicThreadMain);
    receiveThread = std::thread(receiveThreadMain);

  }
}
}


extern "C" {
void HAL_CAN_SendMessage(uint32_t messageID, const uint8_t* data,
                         uint8_t dataSize, int32_t periodMs, int32_t* status) {
  std::scoped_lock lock{canSendMutex};
  // TODO: Handle Periodic
  tNiEmbCANFrame frame;
  frame.m_dataLength = dataSize;
  std::copy_n(data, dataSize, std::begin(frame.m_payload));

  if (messageID & HAL_CAN_IS_FRAME_11BIT) {
    frame.m_isExtended = false;
    frame.m_identifier = messageID & kNiEmbCANMaxStandardID;
  } else {
    frame.m_isExtended = true;
    frame.m_identifier = messageID & kNiEmbCANMaxExtendedID;
  }
  frame.m_isRemote = messageID & HAL_CAN_IS_FRAME_REMOTE;

  *status = niEmbCANWrite(canHandle, &frame);
}


void HAL_CAN_ReceiveMessage(uint32_t* messageID, uint32_t messageIDMask,
                            uint8_t* data, uint8_t* dataSize,
                            uint32_t* timeStamp, int32_t* status) {
  std::scoped_lock lock{canReceiveMutex};
  auto msgId = *messageID;
  auto found = std::find_if(canReceivedMessages.begin(), canReceivedMessages.end(), [&](const CANMessage& msg){
      return (msg.messageID & messageIDMask) == msgId && msg.dataSize >= 0;
    });

    if (found == canReceivedMessages.end()) {
      *status = HAL_ERR_CANSessionMux_MessageNotFound;
      return;
    }

    *messageID = found->messageID;
    *dataSize = found->dataSize;
    *timeStamp = found->timeStamp;
    *status = 0;
    std::copy_n(found->data, 8, data);
    found->dataSize = -1;


}

void HAL_CAN_GetCANStatus(float* percentBusUtilization, uint32_t* busOffCount,
                          uint32_t* txFullCount, uint32_t* receiveErrorCount,
                          uint32_t* transmitErrorCount, int32_t* status) {
}
}
