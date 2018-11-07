#ifndef OUTBOUND_H
#define OUTBOUND_H

#include "Arduino.h"
#include "MessageStatus.h"
#include "Message.h"

namespace SC {

class Outbound
{
public:
  Outbound(const Message* Message, unsigned long SequenceNumber, bool ReceiptRequired, MessageStatus* Tracker);
  ~Outbound();

  void Sent();
  void UpdateTracker(MessageStatus Status);
  bool TimeoutElapsed(unsigned long Timeout);
  bool ContinueToSend(byte SendLimit);

  const Message* const pMessage();
  unsigned long pSequenceNumber();
  bool pReceiptRequired();
  byte pNSent();

private:
  const Message* mMessage;
  unsigned long mSequenceNumber;
  bool mReceiptRequired;
  MessageStatus* mTracker;

  unsigned long mSentTimestamp;
  byte mNSent;
};

}

#endif // OUTBOUND_H
