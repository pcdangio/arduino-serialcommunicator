#include "Communicator.h"

using namespace SC;

// CONSTRUCTORS
Communicator::Communicator(long BaudRate)
{
  // Setup the serial port.
  Serial.begin(BaudRate);

  // Initialize parameters to default values.
  Communicator::mQSize = 20;
  Communicator::mSequenceCounter = 0;
  Communicator::mReceiptTimeout = 100;
  Communicator::mTransmitLimit = 5;

  // Set up queues.
  Communicator::mTXQ = new Outbound*[Communicator::mQSize];
  Communicator::mRXQ = new Inbound*[Communicator::mQSize];
  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    Communicator::mTXQ[i] = NULL;
    Communicator::mRXQ[i] = NULL;
  }
}
Communicator::~Communicator()
{
  // Clean out the queues.
  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    if(Communicator::mTXQ[i] != NULL)
    {
      delete Communicator::mTXQ[i];
    }
    if(Communicator::mRXQ[i] != NULL)
    {
      delete Communicator::mRXQ[i];
    }
  }
  delete [] Communicator::mTXQ;
  delete [] Communicator::mRXQ;
}

// METHODS
bool Communicator::Send(const Message* Message, bool ReceiptRequired, MessageStatus* Tracker)
{
  // Find an open spot in the TX queue.
  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    if(Communicator::mTXQ[i] == NULL)
    {
      // An open space was found.
      // Create a new outgoing message in the opening.  It's tracker status is automatically set to queued.
      // Add the sequence number and increment it.
      Communicator::mTXQ[i] = new Outbound(Message, Communicator::mSequenceCounter++, ReceiptRequired, Tracker);

      // Message was successfully added to the queue.
      return true;
    }
  }

  // If this point reached, no spot was found and the message was not added to the outgoing queue.
  return false;
}
unsigned int Communicator::MessagesAvailable()
{
  // Count the amount of non-null ptrs in the RXQ.
  unsigned int Output = 0;
  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    if(Communicator::mRXQ[i] != NULL)
    {
      Output++;
    }
  }
  return Output;
}
const Message* Communicator::Receive()
{
  // Iterate through the RX queue to find the message with the highest priority and lowest sequence number.
  Inbound* ToRead = NULL;
  unsigned int RXQLocation = 0;

  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    // Check to see if there is anything at this position in the RX queue.
    if(Communicator::mRXQ[i] != NULL)
    {
      // Check to see if ToRead is empty.
      if(ToRead == NULL)
      {
        // Initialize ToRead with the message at this location.
        ToRead = Communicator::mRXQ[i];
        RXQLocation = i;
      }
      else
      {
        // Compare ToRead with the inbound message at this location.
        if(Communicator::mRXQ[i]->pMessage()->pPriority() > ToRead->pMessage()->pPriority())
        {
          // This location in the RXQ has a higher priority.  Set ToRead.
          ToRead = Communicator::mRXQ[i];
          RXQLocation = i;
        }
        else if(Communicator::mRXQ[i]->pMessage()->pPriority() == ToRead->pMessage()->pPriority())
        {
          // Proirities are the same.  Choose the inbound message with the earlier sequence number.
          if(Communicator::mRXQ[i]->pSequenceNumber() < ToRead->pSequenceNumber())
          {
            // This location has the same priority but a lower sequence number.  Set ToRead.
            ToRead = Communicator::mRXQ[i];
            RXQLocation = i;
          }
        }
      }
    }
  }

  // Create a copy of the message pointer to return.
  const Message* Output = ToRead->pMessage();

  // Remove the inbound message from the queue.
  delete Communicator::mRXQ[RXQLocation];
  Communicator::mRXQ[RXQLocation] = NULL;

  // Return the message.
  return Output;
}
const Message* Communicator::Receive(unsigned int ID)
{
  // Iterate through the RX queue to find the specified message with the highest priority and lowest sequence number.
  Inbound* ToRead = NULL;
  unsigned int RXQLocation = 0;

  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    // Check to see if there is anything at this position in the RX queue.
    // Also check to see if the message matches the ID.
    if(Communicator::mRXQ[i] != NULL && Communicator::mRXQ[i]->pMessage()->pID() == ID)
    {
      // Check to see if ToRead is empty.
      if(ToRead == NULL)
      {
        // Initialize ToRead with the message at this location.
        ToRead = Communicator::mRXQ[i];
        RXQLocation = i;
      }
      else
      {
        // Compare ToRead with the inbound message at this location.
        if(Communicator::mRXQ[i]->pMessage()->pPriority() > ToRead->pMessage()->pPriority())
        {
          // This location in the RXQ has a higher priority.  Set ToRead.
          ToRead = Communicator::mRXQ[i];
          RXQLocation = i;
        }
        else if(Communicator::mRXQ[i]->pMessage()->pPriority() == ToRead->pMessage()->pPriority())
        {
          // Proirities are the same.  Choose the inbound message with the earlier sequence number.
          if(Communicator::mRXQ[i]->pSequenceNumber() < ToRead->pSequenceNumber())
          {
            // This location has the same priority but a lower sequence number.  Set ToRead.
            ToRead = Communicator::mRXQ[i];
            RXQLocation = i;
          }
        }
      }
    }
  }

  // Create a copy of the message pointer to return.
  const Message* Output = ToRead->pMessage();

  // Remove the inbound message from the queue.
  delete Communicator::mRXQ[RXQLocation];
  Communicator::mRXQ[RXQLocation] = NULL;

  // Return the message.
  return Output;
}
void Communicator::Spin()
{
  // First send messages.
  Communicator::SpinTX();

  // Next, receive messages.
  Communicator::SpinRX();
}

void Communicator::SpinTX()
{
  // Send messages.
  // Scan through the entire TXQ to find the message with the highest priority and lowest sequence number, but is also not awaiting a timestamp.
  Outbound* ToSend = NULL;
  unsigned int TXQLocation = 0;
  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    // Check if this address has a valid outbound message in it.
    if(Communicator::mTXQ[i] != NULL)
    {
      // There is an outbound in this position.
      // Check if this position is a receipt required message stuck waiting for a timeout.
      if(Communicator::mTXQ[i]->pReceiptRequired() && !Communicator::mTXQ[i]->TimeoutElapsed(Communicator::mReceiptTimeout))
      {
        // Skip this position.
        continue;
      }
      // Check if ToSend has a value in it yet.
      if(ToSend == NULL)
      {
        // Update ToSend to the first available value.
        ToSend = Communicator::mTXQ[i];
        // Update location so this outbound message can be removed from the queue at the end.
        TXQLocation = i;
      }
      else
      {
        // Compare ToSend with the current queued message.
        if(Communicator::mTXQ[i]->pMessage()->pPriority() > ToSend->pMessage()->pPriority())
        {
          // Update ToSend to this higher priority outbound message.
          ToSend = Communicator::mTXQ[i];
          // Update location so message can be removed from queue at the end.
          TXQLocation = i;
        }
        else if(Communicator::mTXQ[i]->pMessage()->pPriority() == ToSend->pMessage()->pPriority())
        {
          // Priorites are the same.  Find the earliest sequence number.  Don't have to worry about overflow here since messages are sent automatically shortly after queued.
          if(Communicator::mTXQ[i]->pSequenceNumber() < ToSend->pSequenceNumber())
          {
            // Update ToSend to this lower sequence number outbound message.
            ToSend = Communicator::mTXQ[i];
            // Update location so message can be removed from queue at the end.
            TXQLocation = i;
          }
        }
      }
    }
  }

  Serial.println("spin");


  // At this point, ToSend contains the highest priority, lowest sequence message that is ready to be sent (e.g. not waiting for a timeout).
  // Step 1: Check if there is anything that needs to be sent.
  if(ToSend != NULL)
  {
    // Step 2: Check if this is the first time the message will be sent.
    if(ToSend->pNTransmissions() == 0)
    {
      // Message has not been sent yet.
      // Step 2.A.1: Send the message.
      Communicator::TX(ToSend);
      // Step 2.A.2: Check if receipt is required.
      if(ToSend->pReceiptRequired())
      {
        // Receipt is required.
        // Step 2.A.2.A.1: Leave in the TXQ, and update the tracker status.
        ToSend->UpdateTracker(MessageStatus::Verifying);
        Serial.println("First Send: Receipt Required");
      }
      else
      {
        // Receipt is not required.
        // Step 2.A.2.B.1: Update tracker status status to sent.
        ToSend->UpdateTracker(MessageStatus::Sent);
        // Step 2.A.2.B.2: Remove this outbound message from the queue.
        delete Communicator::mTXQ[TXQLocation];
        Communicator::mTXQ[TXQLocation] = NULL;
        Serial.println("First Send: Receipt Not Required");
      }
    }
    else
    {
      // Message has been sent at least once.
      // Step 2.B.1: Check the timout.
      if(ToSend->TimeoutElapsed(Communicator::mReceiptTimeout))
      {
        // Timeout has elapsed.
        Serial.println("Timeout Elapsed");
        // Step 2.B.1.A.1: Check if the message can be resent.
        if(ToSend->CanRetransmit(Communicator::mTransmitLimit))
        {
          // Message has not hit the maximum send limit.
          // Step 2.B.1.A.1.A.1: Resend the message.
          Communicator::TX(ToSend);
          Serial.println("Message Resent.");
        }
        else
        {
          // Message has been sent the maximum number of times.
          // Step 2.B.1.A.1.B.1: Update tracker status.
          ToSend->UpdateTracker(MessageStatus::NotReceived);
          // Step 2.B.1.A.1.B.2: Remove from the TXQ.
          delete Communicator::mTXQ[TXQLocation];
          Communicator::mTXQ[TXQLocation] = NULL;
          Serial.println("Max retries reached");
        }
      }
      // Otherwise don't do anything.  The else case for 2.B.1 should never be reached since messages waiting for timeout are skipped.
    }
  }
}
void Communicator::SpinRX()
{

}

void Communicator::TX(Outbound* Message)
{


  // CALL Sent() METHOD ON THE MESSAGE TO UPDATE COUNTERS!


  // Send the message.
    byte* Array = new byte[Message->pMessage()->pMessageLength()];
    Message->pMessage()->Serialize(Array, 0);
    for(unsigned long i = 0; i < Message->pMessage()->pMessageLength(); i++)
    {
      Serial.print(Array[i], HEX);
    }
    Serial.println();
    Message->Sent();
}

// PROPERTIES
//unsigned int Communicator::pQueueSize();
//void Communicator::pQueueSize(unsigned int Length);
//unsigned long Communicator::pReceiptTimeout();
//void Communicator::pReceiptTimeout(unsigned long Timeout);
//unsigned int Communicator::pMaxRetries();
//void Communicator::pMaxRetries(unsigned int Retries);
