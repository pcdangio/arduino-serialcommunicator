#ifndef MESSAGESTATUS_H
#define MESSAGESTATUS_H

namespace SC {

enum MessageStatus
{
  Queued = 0,
  Sent = 1,
  Verifying = 2,
  Received = 3,
  NotReceived = 4
};

}

#endif // MESSAGESTATUS_H
