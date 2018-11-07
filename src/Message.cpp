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
}
Message::Message(const byte* ByteArray, unsigned long Address)
{
  // Parse out ID, priority, and data length.
  Message::mID = Message::Deserialize<unsigned int>(ByteArray, Address);
  Message::mPriority = Message::Deserialize<byte>(ByteArray, Address + 2);
  Message::mDataLength = Message::Deserialize<unsigned int>(ByteArray, Address + 3);
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
template <typename T>
bool Message::SetData(unsigned int Address, T Data)
{
  // First check we are operating within the data array bounds.
  if(Address + sizeof(Data) - 1 >= Message::mDataLength)
  {
    return false;
  }

  Message::Serialize<T>(Message::mData, Address, Data);

  // Return success.
  return true;
}
template <typename T>
T Message::GetData(unsigned int Address) const
{
  return Message::Deserialize<T>(Message::mData, Address);
}
void Message::Serialize(byte* ByteArray, unsigned long Address) const
{
  // Serialize the message into the byte array.
  Message::Serialize<unsigned int>(ByteArray, Address, Message::mID);
  Message::Serialize<byte>(ByteArray, Address + 2, Message::mPriority);
  Message::Serialize<unsigned int>(ByteArray, Address + 3, Message::mDataLength);
  for(unsigned int i = 0; i < Message::mDataLength; i++)
  {
    ByteArray[Address + 5 + i] = Message::mData[i];
  }
}

template <typename T>
void Message::Serialize(byte* Array, unsigned long Address, T Data)
{
  // Recast the data into a byte array.
  const byte* Bytes = reinterpret_cast<const byte*>(&Data);
  // Copy the bytes into the data array.
  for(unsigned int i = 0; i < sizeof(Data); i++)
  {
    Array[Address + i] = Bytes[i];
  }
}
template <typename T>
T Message::Deserialize(const byte* Array, unsigned long Address)
{
  return *(reinterpret_cast<const T*>(&Array[Address]));
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
