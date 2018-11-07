#include "Outbound.h"

using namespace SC;

// CONSTRUCTORS
Outbound::Outbound(const SC::Message* Message, unsigned long SequenceNumber, bool ReceiptRequired, MessageStatus* Tracker)
{
  // Store locals.
  Outbound::mMessage = Message;
  Outbound::mSequenceNumber = SequenceNumber;
  Outbound::mReceiptRequired = ReceiptRequired;
  Outbound::mTracker = Tracker;

  // Initialize counters.
  Outbound::mTransmitTimestamp = 0;
  Outbound::mNTransmissions = 0;

  // Set tracker status to queued.
  Outbound::UpdateTracker(MessageStatus::Queued);
}
Outbound::~Outbound()
{
  // Delete the message since nobody will need it again.
  delete Outbound::mMessage;
}

// METHODS
void Outbound::Sent()
{
  // Update Transmission timestamp.
  Outbound::mTransmitTimestamp = millis();
  // Update Transmission counter.
  Outbound::mNTransmissions++;
}
void Outbound::UpdateTracker(MessageStatus Status)
{
  // Check if a tracker was supplied.
  if(Outbound::mTracker != NULL)
  {
    (*Outbound::mTracker) = Status;
  }
}
bool Outbound::TimeoutElapsed(unsigned long Timeout)
{
  unsigned long CurrentTime = millis();
  // Check for wrapping around max value of unsigned long.
  if(Outbound::mTransmitTimestamp <= CurrentTime)
  {
    // No wrapping occured.
    return ((CurrentTime - Outbound::mTransmitTimestamp) > Timeout);
  }
  else
  {
    // Wrapping occured.
    return ((((unsigned long)(0xFFFFFFFF) - Outbound::mTransmitTimestamp) + CurrentTime + 1) > Timeout);
  }
}
bool Outbound::CanRetransmit(byte TransmitLimit)
{
  return Outbound::mNTransmissions < TransmitLimit;
}

// PROPERTIES
const Message* const Outbound::pMessage()
{
  return Outbound::mMessage;
}
unsigned long Outbound::pSequenceNumber()
{
  return Outbound::mSequenceNumber;
}
bool Outbound::pReceiptRequired()
{
  return Outbound::mReceiptRequired;
}
byte Outbound::pNTransmissions()
{
  return Outbound::mNTransmissions;
}
