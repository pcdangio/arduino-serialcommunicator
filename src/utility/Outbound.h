/// \file Outbound.h
/// \brief Defines the SC::Outbound class.
#ifndef OUTBOUND_H
#define OUTBOUND_H

#include "Arduino.h"
#include "MessageStatus.h"
#include "Message.h"

namespace SC {

///
/// \brief Provides management of outbound messages.
///
class Outbound
{
public:
    ///
    /// \brief Creates a new Outbound message instance.
    /// \param Message A pointer to the message that will be sent.
    /// \param SequenceNumber The sequence number assigned by the transmitting SC::Communicator.
    /// \param ReceiptRequired Indicates if receipt is required for this message.
    /// \param Tracker A pointer to the external tracker for providing message status updates.
    /// \note This class takes control of the SC::Message pointer.
    ///
    Outbound(const Message* Message, unsigned long SequenceNumber, bool ReceiptRequired, MessageStatus* Tracker);
    ~Outbound();

    ///
    /// \brief Instructs the outgoing message that it has been sent.
    /// \details Call this method any time it's message is transmitted.
    /// This informs the Outbound instance to update counters and timestamps related to retransmissions.
    ///
    void Sent();
    ///
    /// \brief UpdateTracker Updates the external tracker with a new message status.
    /// \param Status The new status of the message.
    ///
    void UpdateTracker(MessageStatus Status);
    ///
    /// \brief TimeoutElapsed Checks if a specified timeout has elapsed since the message was last sent.
    /// \param Timeout The length of the timeout period in milliseconds.
    /// \return TRUE if the timeout has been elapsed, otherwise FALSE.
    ///
    bool TimeoutElapsed(unsigned long Timeout);
    ///
    /// \brief CanRetransmit Checks if the message can be retransmitted.
    /// \param TransmitLimit The total number of times a message can be transmitted while attempting to get a receipt.
    /// \return Returns TRUE if the message may be retransmitted, otherwise FALSE.
    ///
    bool CanRetransmit(byte TransmitLimit);

    ///
    /// \brief pMessage PROPERTY Gets a constant pointer to the outbound message.
    /// \return A constant pointer to the constant outbound message.
    ///
    const Message* const pMessage();
    ///
    /// \brief pSequenceNumber PROPERTY Gets the sequence number set by the transmitting SC::Communicator.
    /// \return The sequence number.
    ///
    unsigned long pSequenceNumber();
    ///
    /// \brief pReceiptRequired PROPERTY Gets if the outbound message requires a receipt from the receiving SC::Communicator.
    /// \return Returns TRUE if receipt is required, and FALSE if receipt is not required.
    ///
    bool pReceiptRequired();
    ///
    /// \brief pNTransmissions Gets the total number of times that the message was transmitted.
    /// \return The total number of times that the message was transmitted.
    ///
    byte pNTransmissions();

private:
    ///
    /// \brief mMessage Stores a local copy of the outbound message.
    ///
    const Message* mMessage;
    ///
    /// \brief mSequenceNumber Stores a local copy of the outbound message's sequence number.
    ///
    unsigned long mSequenceNumber;
    ///
    /// \brief mReceiptRequired Flag indicating if receipt is required for the outgoing message.
    ///
    bool mReceiptRequired;
    ///
    /// \brief mTracker Stores a pointer to a tracker to provide message status updates to external code.
    ///
    MessageStatus* mTracker;

    ///
    /// \brief mTransmitTimestamp Stores the last time the message was transmitted.
    ///
    unsigned long mTransmitTimestamp;
    ///
    /// \brief mNTransmissions Stores the total number of times the message was transmitted.
    ///
    byte mNTransmissions;
};

}

#endif // OUTBOUND_H
