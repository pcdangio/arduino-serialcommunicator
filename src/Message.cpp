#include "Message.h"

using namespace SC;

// CONSTRUCTORS
Message::Message(unsigned int ID)
{
  Message::mID = ID;
  Message::mPriority = 0;
  Message::mDataLength = 0;
  Message::mData = NULL;
}
Message::Message(unsigned int ID, unsigned int DataLength)
{
  Message::mID = ID;
  Message::mPriority = 0;
  Message::mDataLength = DataLength;
  Message::mData = new byte[DataLength];
  // Initialize data as zeros.
  for(unsigned int i = 0; i < DataLength; ++i)
  {
    Message::mData[i] = 0;
  }
}
Message::Message(const byte* ByteArray, unsigned long Address)
{
  // Parse out ID, priority, and data length.
  Message::mID = SC::Deserialize<unsigned int>(ByteArray, Address);
  Message::mPriority = SC::Deserialize<byte>(ByteArray, Address + 2);
  Message::mDataLength = SC::Deserialize<unsigned int>(ByteArray, Address + 3);
  // Copy data bytes.
  Message::mData = new byte[Message::mDataLength];
  for(unsigned int i = 0; i < Message::mDataLength; i++)
  {
    Message::mData[i] = ByteArray[Address + 5 + i];
  }
}
Message::~Message()
{
  delete [] Message::mData;
}

// METHODS
void Message::Serialize(byte* ByteArray, unsigned long Address) const
{
  // Serialize the message into the byte array.
  SC::Serialize<unsigned int>(ByteArray, Address, Message::mID);
  SC::Serialize<byte>(ByteArray, Address + 2, Message::mPriority);
  SC::Serialize<unsigned int>(ByteArray, Address + 3, Message::mDataLength);
  for(unsigned int i = 0; i < Message::mDataLength; i++)
  {
    ByteArray[Address + 5 + i] = Message::mData[i];
  }
}

// PROPERTIES
void Message::pID(unsigned int ID)
{
  Message::mID = ID;
}
unsigned int Message::pID() const
{
  return Message::mID;
}
void Message::pPriority(byte Priority)
{
  Message::mPriority = Priority;
}
byte Message::pPriority() const
{
  return Message::mPriority;
}
unsigned int Message::pDataLength() const
{
  return Message::mDataLength;
}
unsigned long Message::pMessageLength() const
{
  // Length is ID(2) + Priority(1) + DataLengthIndicator(2) + DataLength(n)
  return 5 + Message::mDataLength;
}
