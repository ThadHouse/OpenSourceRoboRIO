#ifndef __niEmbcan_h__
#define __niEmbcan_h__

#include <stdint.h>

#ifdef WIN32
  #define  _NIEMBCANFUNC __cdecl
#else
  #define  _NIEMBCANFUNC
#endif

typedef uint32_t tNiEmbCANHandle;
typedef int32_t  tNiEmbCANStatus;
typedef uint64_t tNiEmbCANTimestamp;

#define INVALID_EMBCAN_HANDLE_VALUE 0

#define kNiEmbCANMaxStandardID 0x7FF
#define kNiEmbCANMaxExtendedID 0x1FFFFFFF

// customized baud rate format: 0x8ABCDDDD
// A: sjw   B: tseg2   C: tesg1   DDDD: tq
#define kNiEmbCANCustomBaudRateMask       0x80000000
#define kNiEmbCANCustomBaudRateSjwMask    0x0F000000
#define kNiEmbCANCustomBaudRateSjwShift   24
#define kNiEmbCANCustomBaudRateTSeg2Mask  0x00F00000
#define kNiEmbCANCustomBaudRateTSeg2Shift 20
#define kNiEmbCANCustomBaudRateTSeg1Mask  0x000F0000
#define kNiEmbCANCustomBaudRateTSeg1Shift 16
#define kNiEmbCANCustomBaudRateTqMask     0x0000FFFF
#define kNiEmbCANCustomBaudRateTqShift    0

///////////////////////////////////////////////////////////////////////////////
//
// CAN Frame Type
//
///////////////////////////////////////////////////////////////////////////////
#define kNiEmbCANFrameMaxDataLength 8
typedef struct
{
   uint32_t        m_identifier;
   uint8_t         m_isRemote;
   uint8_t         m_isExtended;
   uint8_t         m_dataLength;
   uint8_t         m_payload[kNiEmbCANFrameMaxDataLength];
} tNiEmbCANFrame;

typedef enum
{
   kNiEmbCANPropertyAutoStart = 0,
   kNiEmbCANPropertyBaudRate,
   kNiEmbCANPropertyListenOnly,
   kNiEmbCANPropertyReceiveQueueSize,
   kNiEmbCANPropertyTransceiverState,
   kNiEmbCANPropertyCommunicationState,
   kNiEmbCANPropertyReceiveErrorCounter,
   kNiEmbCANPropertyTransmitErrorCounter
} tNiEmbCANProperty;

typedef enum
{
   kNiEmbCANTransceiverStateNormal = 0,
   kNiEmbCANTransceiverStateSleep
} tNiEmbCANTransceiverState;

typedef enum
{
   kNiEmbCANCommunicationStateErrorActive = 0,
   kNiEmbCANCommunicationStateErrorPassive,
   kNiEmbCANCommunicationStateBusOff,
   kNiEmbCANCommunicationStateInit
} tNiEmbCANCommunicationState;

typedef struct
{
   uint32_t        m_identifier;
   uint32_t        m_identifierMask;
   uint8_t         m_isExtended;
   uint8_t         m_isRemote;
   uint8_t         m_remoteMask;
} tNiEmbFilterSetting;

typedef enum
{
   kNiEmbCANFilterMode_Disabled = 0,
   kNiEmbCANFilterMode_32bit,
   kNiEmbCANFilterMode_16bit,
   kNiEmbCANFilterMode_8bit
} tNiEmbCANFilterMode;

#define kFilterMaskMatchAll 0xFFFFFFFF


   /* Needed for C++ to C (DLL) calls  */
#ifdef __cplusplus
extern "C" {
#endif
////////////////////////////////////////////////////////////////////////////////
//
// Interface definition
//
////////////////////////////////////////////////////////////////////////////////

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANOpenSession (
   uint32_t     portIndex,
   uint32_t     baudRate,
   uint32_t     recvQueueSize,
   tNiEmbCANHandle * pSessionHandle);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANStart (
   tNiEmbCANHandle sessionHandle);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANStop (
   tNiEmbCANHandle sessionHandle);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANRead (
   tNiEmbCANHandle  sessionHandle,
   int32_t          timeoutInUs,
   tNiEmbCANFrame * pCANFrame,
   tNiEmbCANTimestamp *  pTimeStamp);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANWrite (
   tNiEmbCANHandle   sessionHandle,
   const tNiEmbCANFrame *  pCANFrame);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANCloseSession (
   tNiEmbCANHandle sessionHandle);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANFlush (
   tNiEmbCANHandle sessionHandle);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANWaitForTransmitComplete (
   tNiEmbCANHandle sessionHandle,
   int32_t         timeoutInUs);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANWaitForRemoteWakeup (
   tNiEmbCANHandle sessionHandle,
   int32_t         timeoutInUs);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANWaitForCommunicating (
   tNiEmbCANHandle sessionHandle,
   int32_t         timeoutInUs);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANSetFilterMode (
   tNiEmbCANHandle sessionHandle,
   tNiEmbCANFilterMode  filterMode);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANSetFilterValue (
   tNiEmbCANHandle sessionHandle,
   uint32_t        filterIndex,
   const tNiEmbFilterSetting *  pFilterSetting);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANGetProperty (
   tNiEmbCANHandle sessionHandle,
   tNiEmbCANProperty   propertyID,
   uint32_t   * pPropertyValue);

tNiEmbCANStatus _NIEMBCANFUNC niEmbCANSetProperty (
   tNiEmbCANHandle sessionHandle,
   tNiEmbCANProperty propertyID,
   uint32_t   propertyValue);

#ifdef __cplusplus
   /* See top of header file.  */
}
#endif // __cplusplus

#endif // __niEmbcan_h__
