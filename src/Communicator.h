#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "Arduino.h"

#include "Message.h"
#include "utility/MessageStatus.h"
#include "utility/Inbound.h"
#include "utility/Outbound.h"

namespace SC {

class Communicator
{
public:
  // CONSTRUCTORS
  Communicator(long BaudRate);
  ~Communicator();

  // METHODS
  bool Send(const Message* Message, bool ReceiptRequired = false, MessageStatus* Tracker = NULL);
  unsigned int MessagesAvailable();
  const Message* Receive();
  const Message* Receive(unsigned int ID);
  void Spin();

  // PROPERTIES
  unsigned int pQueueLength();
  void pQueueLength(unsigned int Length);
  unsigned long pReceiptTimeout();
  void pReceiptTimeout(unsigned long Timeout);
  unsigned int pRetries();
  void pRetries(unsigned int Retries);

private:
  unsigned int mQLength;
  unsigned long mSequenceCounter;
  unsigned long mReceiptTimeout;
  byte mSendLimit;

  const Outbound** mTXQ;
  const Inbound** mRXQ;

  void SpinTX();
  void SpinRX();

  void TX(Outbound* Message);
};

}


#endif // COMMUNICATOR_H
