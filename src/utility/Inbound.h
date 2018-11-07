/// \file Inbound.h
/// \brief Defines the SC::Inbound class.
#ifndef INBOUND_H
#define INBOUND_H

#include "Arduino.h"
#include "Message.h"

namespace SC {

///
/// \brief Provides management of inbound messages.
///
class Inbound
{
public:
    ///
    /// \brief Inbound Creates a new inbound message instance.
    /// \param Message A pointer to the SC::Message received via serial.
    /// \param SequenceNumber The sequence number set by the sending SC::Communicator.
    ///
    Inbound(const Message* Message, unsigned long SequenceNumber);

    ///
    /// \brief pMessage PROPERTY Gets a constant copy of the inbound SC::Message.
    /// \return A constant SC::Message copy.
    ///
    const Message* pMessage();
    ///
    /// \brief pSequenceNumber PROPERTY Gets the sequence number of the message set by the sending SC::Communicator.
    /// \return The sequence number of the message.
    ///
    unsigned long pSequenceNumber();

private:
    ///
    /// \brief mMessage Stores a local pointer to the message.
    ///
    const Message* mMessage;
    ///
    /// \brief mSequenceNumber Stores a local copy of the message's originating sequence number.
    ///
    unsigned long mSequenceNumber;
};

}

#endif // INBOUND_H
