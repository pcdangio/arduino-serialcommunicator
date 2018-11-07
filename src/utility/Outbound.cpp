#include "Outbound.h"

using namespace SC;

// CONSTRUCTORS
Outbound::Outbound(const Message* Message, unsigned long SequenceNumber, bool ReceiptRequired, MessageStatus* Tracker)
{
  // Store locals.
  Outbound::mMessage = Message;
  Outbound::mSequenceNumber = SequenceNumber;
  Outbound::mReceiptRequired = ReceiptRequired;
  Outbound::mTracker = Tracker;

  // Initialize counters.
  Outbound::mSentTimestamp = 0;
  Outbound::mNSent = 0;

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
  // Update sent timestamp.
  Outbound::mSentTimestamp = millis();
  // Update sent counter.
  Outbound::mNSent++;
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
  if(Outbound::mSentTimestamp <= CurrentTime)
  {
    // No wrapping occured.
    return ((CurrentTime - Outbound::mSentTimestamp) > Timeout);
  }
  else
  {
    // Wrapping occured.
    return ((((unsigned long)(0xFFFFFFFF) - Outbound::mSentTimestamp) + CurrentTime + 1) > Timeout);
  }
}
bool Outbound::ContinueToSend(byte SendLimit)
{
  return Outbound::mNSent < SendLimit;
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
byte Outbound::pNSent()
{
  return Outbound::mNSent;
}
