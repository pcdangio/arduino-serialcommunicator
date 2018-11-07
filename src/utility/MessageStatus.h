/// \file MessageStatus.h
/// \brief Defines the SC::MessageStatus enumeration.
#ifndef MESSAGESTATUS_H
#define MESSAGESTATUS_H

namespace SC {

///
/// \brief Enumerates the various message states.
///
enum MessageStatus
{
  Queued = 0,       ///< The message has been queued, but has not yet been sent.
  Sent = 1,         ///< The message has been sent, and no receipt was required.
  Verifying = 2,    ///< The message has been sent, and the SC::Communicator is verifying that the message was received.
  Received = 3,     ///< The message was sent, and was verified as received from the receiving SC::Communicator.
  NotReceived = 4   ///< The message was sent, but no verification was received.
};

}

#endif // MESSAGESTATUS_H
