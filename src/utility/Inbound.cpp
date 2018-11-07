#include "Inbound.h"

using namespace SC;

// CONSTRUCTORS
Inbound::Inbound(const Message* Message, unsigned long SequenceNumber)
{
  Inbound::mMessage = Message;
  Inbound::mSequenceNumber = SequenceNumber;
}

// PROPERTIES
const Message* Inbound::pMessage()
{
  return Inbound::mMessage;
}
unsigned long Inbound::pSequenceNumber()
{
  return Inbound::mSequenceNumber;
}
