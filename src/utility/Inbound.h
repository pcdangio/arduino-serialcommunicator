#ifndef INBOUND_H
#define INBOUND_H

#include "Arduino.h"
#include "Message.h"

namespace SC {

class Inbound
{
public:
  Inbound(const Message* Message, unsigned long SequenceNumber);

  const Message* pMessage();
  unsigned long pSequenceNumber();

private:
  const Message* mMessage;
  unsigned long mSequenceNumber;
};

}

#endif // INBOUND_H
