#include "Communicator.h"

#include "utility/Serialization.h"

using namespace SC;

// CONSTRUCTORS
Communicator::Communicator(long BaudRate)
{
  // Setup the serial port.
  Serial1.begin(BaudRate);

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
    // Delete the message.
    delete Message;
    // Return false.
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
const Message* Communicator::Receive(unsigned int ID)
{
  // Iterate through the RX queue to find the specified message with the highest priority and lowest sequence number.
  Inbound* ToRead = NULL;
  unsigned int RXQLocation = 0;

  for(unsigned int i = 0; i < Communicator::mQSize; i++)
  {
    // Check to see if there is anything at this position in the RX queue.
    // Also check to see if the message matches the ID.
    if(Communicator::mRXQ[i] != NULL && (ID == 0xFFFF || Communicator::mRXQ[i]->pMessage()->pID() == ID))
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
      }
      else
      {
        // Receipt is not required.
        // Step 2.A.2.B.1: Update tracker status status to sent.
        ToSend->UpdateTracker(MessageStatus::Sent);
        // Step 2.A.2.B.2: Remove this outbound message from the queue.
        delete Communicator::mTXQ[TXQLocation];
        Communicator::mTXQ[TXQLocation] = NULL;
      }
    }
    else
    {
      // Message has been sent at least once.
      // Step 2.B.1: Check the timout.
      if(ToSend->TimeoutElapsed(Communicator::mReceiptTimeout))
      {
        // Timeout has elapsed.
        // Step 2.B.1.A.1: Check if the message can be resent.
        if(ToSend->CanRetransmit(Communicator::mTransmitLimit))
        {
          // Message has not hit the maximum send limit.
          // Step 2.B.1.A.1.A.1: Resend the message.
          Communicator::TX(ToSend);
        }
        else
        {
          // Message has been sent the maximum number of times.
          // Step 2.B.1.A.1.B.1: Update tracker status.
          ToSend->UpdateTracker(MessageStatus::NotReceived);
          // Step 2.B.1.A.1.B.2: Remove from the TXQ.
          delete Communicator::mTXQ[TXQLocation];
          Communicator::mTXQ[TXQLocation] = NULL;
        }
      }
      // Otherwise don't do anything.  The else case for 2.B.1 should never be reached since messages waiting for timeout are skipped.
    }
  }
}
void Communicator::SpinRX()
{
    // Look for the header byte.  Read bytes until header is found, timed out, or too many bytes have been read.
    int Read = Serial1.read();
    byte NReads = 0;
    while(Read != Communicator::cHeaderByte && Read != -1 && NReads++ < 32)
    {
        Read = Serial1.read();
    }

    if(Read != Communicator::cHeaderByte)
    {
        // Header byte could not be found in time.  Quit.
        return;
    }

    // If this point is reached, a header byte has been found.
    // Read the next 10 bytes in to ultimately get to the number of data bytes in the message.
    byte* PKTBytes = new byte[11];
    unsigned long PKTLength = 11;
    // Set the first byte of the packet to the header byte.
    PKTBytes[0] = Communicator::cHeaderByte;
    // Read in the next ten bytes.
    if(Communicator::RX(&PKTBytes[1], 10) != 10)
    {
        // Timeout occured.
        return;
    }

    // If this point is reached, the first 11 bytes of the packet have been read.
    // Deserialize NDataBytes.
    unsigned int NDataBytes = SC::Deserialize<unsigned int>(PKTBytes, 9);
    // Resize the PKTBytes to accomodate the databytes + checksum.
    byte* TMPPKT = new byte[PKTLength + NDataBytes + 1];
    for(unsigned long i = 0; i < PKTLength; i++)
    {
        TMPPKT[i] = PKTBytes[i];
    }
    delete [] PKTBytes;
    PKTBytes = TMPPKT;
    PKTLength += NDataBytes + 1;
    // Attempt to read the remainder of the packet.
    if(Communicator::RX(&PKTBytes[11], NDataBytes + 1) != NDataBytes + 1)
    {
        // Timeout occured.
        return;
    }

    // FULL PACKET HAS BEEN READ

    // First, make sure the checksum matches.
    bool ChecksumOK = PKTBytes[PKTLength - 1] == Communicator::Checksum(PKTBytes, PKTLength - 1);
    // Second, get the sequence number from the packet.
    unsigned long SequenceNumber = SC::Deserialize<unsigned long>(PKTBytes, 1);

    // Next, handle receipts.
    switch(Communicator::ReceiptType(PKTBytes[5]))
    {
    case Communicator::ReceiptType::NotRequired:
        // Do nothing.
        break;
    case Communicator::ReceiptType::Required:
        {
            // Send the receipt.
            // Draft message.
            byte* Receipt = new byte[12];
            for(byte i = 0; i < 5; i++)
            {
                Receipt[i] = PKTBytes[i];
            }
            if(ChecksumOK)
            {
                Receipt[5] = (byte)Communicator::ReceiptType::Received;
            }
            else
            {
                Receipt[5] = (byte)Communicator::ReceiptType::ChecksumMismatch;
            }
            for(byte i = 6; i < 9; i++)
            {
                Receipt[i] = PKTBytes[i];
            }
            Receipt[9] = 0;
            Receipt[10] = 0;
            Receipt[11] = Communicator::Checksum(Receipt, 10);
            // Send message.
            Communicator::TX(Receipt, 12);
        }
        break;
    case Communicator::ReceiptType::Received:
        {
            // Remove the associated message from the TXQ if it is still in there.
            for(byte i = 0; i < Communicator::mQSize; i++)
            {
                if(Communicator::mTXQ[i] != NULL && Communicator::mTXQ[i]->pSequenceNumber() == SequenceNumber)
                {
                    // Update the tracker status.
                    Communicator::mTXQ[i]->UpdateTracker(MessageStatus::Received);
                    // Remove from the queue.
                    delete Communicator::mTXQ[i];
                    Communicator::mTXQ[i] = NULL;
                    // Break from the for loop.
                    break;
                }
            }
        }
        break;
    case Communicator::ReceiptType::ChecksumMismatch:
        // Don't do anything.  This can be used in the future (e.g. immediate retransmit from TXQ instead of waiting for timeout).
        break;
    }

    // Lastly, emplace this as an inbound message.
    if(ChecksumOK)
    {
        // Find an open position in the RXQ.
        int Location = -1;
        for(byte i = 0; i < Communicator::mQSize; i++)
        {
            if(Communicator::mRXQ[i] == NULL)
            {
                Location = i;
                break;
            }
        }
        if(Location >= 0)
        {
            // Create the message itself.
            Message* MSG = new Message(PKTBytes, 6);
            // Add a new Inbound to the RXQ.
            Communicator::mRXQ[Location] = new Inbound(MSG, SequenceNumber);
        }
    }

    // Clean up PKTBytes.
    delete PKTBytes;
}

void Communicator::TX(Outbound* Message)
{
    // Grab the original serialized bytes of the message.
    unsigned long MSGLength = Message->pMessage()->pMessageLength();
    byte* MSGBytes = new byte[MSGLength];
    Message->pMessage()->Serialize(MSGBytes);

    // Create packet byte array.
    // Add in the message length + 7 bytes of the packet (1 Header, 4 Sequence, 1 Receipt, 1 Checksum)
    unsigned long PKTLength = MSGLength + 7;
    byte* PKTBytes = new byte[PKTLength];

    // Write the front part of the packet.
    PKTBytes[0] = Communicator::cHeaderByte;
    SC::Serialize<unsigned long>(PKTBytes, 1, Message->pSequenceNumber());
    PKTBytes[5] = byte(Message->pReceiptRequired());

    // Write the message bytes into the packet.
    Message->pMessage()->Serialize(PKTBytes, 6);

    // Calculate the CRC.
    // Use length of PTKLength - 1 because the last position in the array is for the checksum itself.
    PKTBytes[PKTLength - 1] = Communicator::Checksum(PKTBytes, PKTLength - 1);

    // Send the message via Serial1.
    Communicator::TX(PKTBytes, PKTLength);

    // Call the Sent method on the outbound message to update timestamps and counters.
    Message->Sent();

    // Delete the packet and message bytes.
    delete [] MSGBytes;
    delete [] PKTBytes;
}
void Communicator::TX(byte *Packet, unsigned long Length)
{
    // Send header first.
    Serial1.write(Packet[0]);
    // Write the rest of the bytes, with escapement.
    for(unsigned long i = 1; i < Length; i++)
    {
        if(Packet[i] == Communicator::cHeaderByte || Packet[i] == Communicator::cEscapeByte)
        {
            Serial1.write(Communicator::cEscapeByte);
            Serial1.write(Packet[i] - 1);
        }
        else
        {
            Serial1.write(Packet[i]);
        }
    }
}
unsigned long Communicator::RX(byte *Buffer, unsigned long Length)
{
    unsigned long BytesRead = 0;
    // Iterate over the number of bytes.
    for(unsigned long i = 0; i < Length; i++)
    {
        // Read the next byte.
        int Read = Serial1.read();
        switch(Read)
        {
        case -1:
        {
            // Timeout occured.
            return BytesRead;
        }
        case Communicator::cEscapeByte:
        {
            // Escape byte.  Read in the next byte and unescape it.
            int Next = Serial1.read();
            if(Next == -1)
            {
                // Time occured.
                return BytesRead;
            }
            Buffer[i] = Next + 1;
            BytesRead++;
            break;
        }
        default:
        {
            // Regular byte.
            Buffer[i] = Read;
            BytesRead++;
            break;
        }
        }
    }
    return BytesRead;
}
byte Communicator::Checksum(byte *Packet, unsigned long Length)
{
    byte Checksum = 0;
    for(unsigned long i = 0; i < Length; i++)
    {
        Checksum ^= Packet[i];
    }
    return Checksum;
}

// PROPERTIES
unsigned int Communicator::pQueueSize()
{
    return Communicator::mQSize;
}
void Communicator::pQueueSize(unsigned int Length)
{
    // Check if a resize is necessary.
    if(Length != Communicator::mQSize)
    {
        // Need to resize the queues.

        // Create temporary queues.
        Outbound** TMPTXQ = new Outbound*[Length];
        Inbound** TMPRXQ = new Inbound*[Length];

        // Fill the temporary queues.
        for(unsigned int i = 0; i < min(Length, Communicator::mQSize); i++)
        {
            TMPTXQ[i] = Communicator::mTXQ[i];
            TMPRXQ[i] = Communicator::mRXQ[i];
        }

        // Check if the rest of the temporary queue needs to be NULLED out.
        for(unsigned int i = Communicator::mQSize; i < Length; i++)
        {
            TMPTXQ[i] = NULL;
            TMPRXQ[i] = NULL;
        }

        // Replace the queues.
        delete [] Communicator::mTXQ;
        delete [] Communicator::mRXQ;
        Communicator::mTXQ = TMPTXQ;
        Communicator::mRXQ = TMPRXQ;

        // Update the QSize.
        Communicator::mQSize = Length;
    }
}
unsigned long Communicator::pReceiptTimeout()
{
    return Communicator::mReceiptTimeout;
}
void Communicator::pReceiptTimeout(unsigned long Timeout)
{
    Communicator::mReceiptTimeout = Timeout;
}
unsigned int Communicator::pMaxRetries()
{
    return Communicator::mTransmitLimit;
}
void Communicator::pMaxRetries(unsigned int Retries)
{
    Communicator::mTransmitLimit = Retries;
}
