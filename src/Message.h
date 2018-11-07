/// \file Message.h
/// \brief Defines the SC::Message class.
#ifndef MESSAGE_H
#define MESSAGE_H

#include "Arduino.h"

#include "utility/Serialization.h"

namespace SC {

/// \brief Represents a single message of data that can be communicated.
class Message
{
public:
  // CONSTRUCTORS
  /// \brief Creates a new message.
  /// \param ID The ID of the message.
  /// \details This creates a new message that has no data and a default priority of 0.
  Message(unsigned int ID);
  /// \brief Creates a new message that will hold data.
  /// \param ID The ID of the message.
  /// \param DataLength The length of the data that will be included in the message.
  /// \details This creates a new message with data and has a default priority of 0.
  /// Use the SetData method for adding the actual data after instantiation.
  Message(unsigned int ID, unsigned int DataLength);
  /// \brief Deserializes a message from an existing byte array.
  /// \param ByteArray The array that contains the serialized message data.
  /// \param Address OPTIONAL The index in the array where the serialized message starts.
  Message(const byte* ByteArray, unsigned long Address = 0);
  /// \brief Destroys the message instance and cleans up resources.
  ~Message();

  // METHODS
  /// \brief POLYMORPHIC Sets a data field in the message.
  /// \param Address The address of the data field in the message.
  /// \param Data The data to write to the supplied address.
  /// \return TRUE if the write was successful, FALSE if an overrun occured.
  template <typename T>
  bool SetData(unsigned int Address, T Data)
  {
    // First check we are operating within the data array bounds.
    if(Address + sizeof(Data) - 1 >= Message::mDataLength)
    {
      return false;
    }

    SC::Serialize<T>(Message::mData, Address, Data);

    // Return success.
    return true;
  }
  /// \brief POLYMORPHIC Gets a data field from the message.
  /// \param Address The address of the data field in the message.
  /// \return The data from the requested address.
  template <typename T>
  T GetData(unsigned int Address) const
  {
    return SC::Deserialize<T>(Message::mData, Address);
  }
  /// \brief Serializes the message into a supplied byte array for transmission.
  /// \param ByteArray The array to serialize the message into.
  /// \param Address OPTIONAL The address in the array to begin writing to.
  void Serialize(byte* ByteArray, unsigned long Address = 0) const;

  // PROPERTIES
  /// \brief Sets the ID of the message.
  /// \param ID The new message ID.
  void pID(unsigned int ID);
  /// \brief Gets the ID of the message.
  /// \return The ID of the message.
  unsigned int pID() const;
  /// \brief Sets the priority of the message.
  /// \param Priority The new message priority.
  /// \details Larger numbers are higher priority.
  void pPriority(byte Priority);
  /// \brief Gets the priority of the message.
  /// \return The message's priority.
  /// \details Larger numbers are higher priority.
  byte pPriority() const;
  /// \brief Gets the message's data length.
  /// \return The length of the message's data fields in bytes.
  unsigned int pDataLength() const;
  /// \brief Gets the messag's total length.
  /// \return THe total length of the message in bytes.
  unsigned long pMessageLength() const;

private:
  /// \brief Stores the message's ID.
  unsigned int mID;
  /// \brief Stores the message's priority.
  byte mPriority;
  /// \brief Stores the message's data length.
  unsigned int mDataLength;
  /// \brief Stores the message's data bytes.
  byte* mData;
};

}

#endif // MESSAGE_H
