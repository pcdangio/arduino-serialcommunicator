#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "Arduino.h"

#include "Message.h"
/// \file Communicator.h
/// \brief Defines the SC::Communicator class.
#include "utility/MessageStatus.h"
#include "utility/Inbound.h"
#include "utility/Outbound.h"

///
/// \brief Contains all code related to the SerialCommunicator library.
///
namespace SC {

///
/// \brief Facilitates the sending and receiving of messages via serial.
///
class Communicator
{
public:
    // CONSTRUCTORS
    ///
    /// \brief Communicator Creates a new communicator instance.
    /// \param SerialPort The Arduino HardwareSerial port to use for communications.
    /// \note Application must call begin() on serial port before using this class.
    ///
    Communicator(Stream& SerialPort);
    ~Communicator();

    // METHODS
    ///
    /// \brief Send Sends a message.
    /// \param Message The message to send.
    /// \param ReceiptRequired OPTIONAL Indicates that the message should be retransmitted until a receipt is received from the endpoint.  Defaults to FALSE.
    /// \param Tracker OPTIONAL A pointer to a tracker for continuous updates on the sent message's status. Defaults to NULL (e.g. no tracking).
    /// \return Returns TRUE if the message was queued into the TX queue.  Returns FALSE if the TX queue is full.
    /// \details This places a message into the TX queue for sending.  The communicator sends messages from the queue based on highest priority, followed
    /// by earliest.  The calling code can keep track of the message's status using the Tracker parameter.  The Communicator will update the Tracker
    /// pointer as the message's status changes.  Once placed in the queue, the message's status is set to SC::MessageStatus::Queued.
    ///
    bool Send(const Message* Message, bool ReceiptRequired = false, MessageStatus* Tracker = NULL);
    ///
    /// \brief MessagesAvailable Counts the number of messages available to read in the RX queue.
    /// \return The number of available messages.
    ///
    unsigned int MessagesAvailable();
    ///
    /// \brief Receive Receives the next message from the RX queue.
    /// \param ID OPTIONAL The ID of the next message to receive.  Defaults to 0xFFFF, which will receive any ID.
    /// \return A pointer to the received message.  Will return NULL if no messages are available.
    /// \note The calling code shall become responsible for the Message pointer and must clean up the Message's resources.
    /// \details Messages are ordered by highest priority, followed by earliest received.
    ///
    const Message* Receive(unsigned int ID = 0xFFFF);
    ///
    /// \brief Spin Performs the Communicator's regular duties.
    /// \note This should be called regularly in the main loop of your code.
    /// \details A single spin operation will only attempt to send and recieve one Message.
    /// This is to prevent the Spin method from severely blocking the main loop of the calling code.
    ///
    void Spin();

    // PROPERTIES
    ///
    /// \brief pQueueSize PROPERTY Gets the size of the internal TX and RX queues.
    /// \return The size of the internal queues.
    /// \details If the internal TX or RX queues become full, they will not allow any more
    /// messages to be enqueued until space opens up from a Spin() call.  These queue sizes
    /// ensure that the Arduino's memory does not fill up.
    /// \note The default value is 20 messages for each queue.
    ///
    unsigned int pQueueSize();
    ///
    /// \brief pQueueSize PROPERTY Sets the size of the internal TX and RX queues.
    /// \param Size The new size of the queues.
    /// \details If the internal TX or RX queues become full, they will not allow any more
    /// messages to be enqueued until space opens up from a Spin() call.  These queue sizes
    /// ensure that the Arduino's memory does not fill up.
    /// \note The default value is 20 messages for each queue.
    ///
    void pQueueSize(unsigned int Size);
    ///
    /// \brief pReceiptTimeout PROPERTY Gets the length of the receipt timeout in milliseconds.
    /// \return The length of the timeout in milliseconds.
    /// \details When a message is sent with receipt required, the transmittnig Communicator will
    /// wait for the specified timeout to receive a receipt message from the receiving
    /// Communicator.  If the timeout elapses without getting a receipt, the Communicator will
    /// then attempt to retransmit the message and repeat this process.
    /// \note The default value is 100ms.
    ///
    unsigned long pReceiptTimeout();
    ///
    /// \brief pReceiptTimeout PROPERTY Sets the length of the receipt timeout in milliseconds.
    /// \param Timeout The length of the timeout to use in milliseconds.
    /// \details When a message is sent with receipt required, the transmittnig Communicator will
    /// wait for the specified timeout to receive a receipt message from the receiving
    /// Communicator.  If the timeout elapses without getting a receipt, the Communicator will
    /// then attempt to retransmit the message and repeat this process.
    /// \note The default value is 100ms.
    ///
    void pReceiptTimeout(unsigned long Timeout);
    ///
    /// \brief pMaxRetries PROPERTY Gets the total number of times a message can be transmitted
    /// while attempting to get a receipt from the message's endpoint.
    /// \return The maximum amount of message tranmissions.
    /// \details When a message is sent with a receipt required, the Communicator will wait for
    /// a receipt from the message's endpoint.  If no receipt is received, the Communicator will
    /// retransmit the message for the specified number of times before giving up.  If the
    /// maximum number of retries is reached without getting a receipt, the message's tracker
    /// status will be set to SC::MessageStatus::NotReceived.
    /// \note The default value is 5 transmissions.
    ///
    unsigned int pMaxRetries();
    ///
    /// \brief pMaxRetries PROPERTY Sets the total number of times a message can be transmitted
    /// while attempting to get a receipt from the message's endpoint.
    /// \param Retries The maximum amount of message transmissions.
    /// \details When a message is sent with a receipt required, the Communicator will wait for
    /// a receipt from the message's endpoint.  If no receipt is received, the Communicator will
    /// retransmit the message for the specified number of times before giving up.  If the
    /// maximum number of retries is reached without getting a receipt, the message's tracker
    /// status will be set to SC::MessageStatus::NotReceived.
    /// \note The default value is 5 transmissions.
    ///
    void pMaxRetries(unsigned int Retries);

protected:
    // ENUMS
    ///
    /// \brief Enumerates the different levels of the receipt field within a message.
    ///
    enum class ReceiptType
    {
        NotRequired = 0,        ///< In a transmitted message, indicates that no receipt is required from the receiver.
        Required = 1,           ///< In a transmitted message, indicates that a receipt is required from the receiver.
        Received = 2,           ///< In a receipt message, indicates that the message was properly received.
        ChecksumMismatch = 3    ///< In a receipt message, indicates that the message was received, but the checksum did not match.
    };

    // CONSTANTS
    ///
    /// \brief cHeaderByte Stores the header byte flag.
    ///
    static const byte cHeaderByte = 0xAA;
    ///
    /// \brief cEscapeByte Stores the escape byte flag.
    ///
    static const byte cEscapeByte = 0x1B;

    // ATTRIBUTES
    ///
    /// \brief mSerial A reference to the Arduino serial port to use for communication.
    ///
    Stream* mSerial;

    ///
    /// \brief mQSize Stores the size of the TX/RX queues in messages.
    ///
    unsigned int mQSize;
    ///
    /// \brief mSequenceCounter Stores the current sequence number for assigning monotonic IDs to messages.
    ///
    unsigned long mSequenceCounter;
    ///
    /// \brief mReceiptTimeout Stores the receipt timeout in milliseconds.
    ///
    unsigned long mReceiptTimeout;
    ///
    /// \brief mTransmitLimit Stores the max number of transmits.
    ///
    byte mTransmitLimit;

    ///
    /// \brief mTXQ The internal TX queue.
    ///
    Outbound** mTXQ;
    ///
    /// \brief mRXQ The internal RX queue.
    ///
    Inbound** mRXQ;

    // METHODS
    ///
    /// \brief SpinTX Conducts the TX duties during a spin cycle.
    ///
    void SpinTX();
    ///
    /// \brief SpinRX Conducts the RX duties during a spin cycle.
    ///
    void SpinRX();
    ///
    /// \brief TX Facilitates the serialization and sending of a message via serial.
    /// \param Message The outbound message to transmit.
    ///
    void TX(Outbound* Message);
    ///
    /// \brief TX Writes the specified bytes to the serial buffer.
    /// \param Packet The fully formed packet to write.
    /// \param Length The length of the packet to write.
    /// \details This will send bytes with proper escapement.
    ///
    void TX(byte* Packet, unsigned long Length);
    ///
    /// \brief RX Reads the specified amount of bytes from the serial buffer.
    /// \param Buffer The pre-allocated buffer to store the read bytes in.
    /// \param Length The number of bytes to read.
    /// \return The number of bytes read.  This can be less than the requested amount if a timeout occurs.
    /// \details This method will read bytes and correct for escape bytes.
    ///
    unsigned long RX(byte* Buffer, unsigned long Length);
    ///
    /// \brief Checksum Calculates the XOR checksum of the provided data array.
    /// \param Array The array of data to calculate the checksum for.
    /// \param Length The length of the data array.
    /// \return The XOR checksum of the data.
    ///
    byte Checksum(byte* Array, unsigned long Length);
};

}


#endif // COMMUNICATOR_H
