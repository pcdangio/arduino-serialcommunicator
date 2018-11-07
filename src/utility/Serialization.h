/// \file Serialization.h
/// \brief Defines serialization and deserialization methods.
#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "Arduino.h"

namespace SC {

/// \brief Serializes any data type into a specified array.
/// \param Array The array to serialize the data into.
/// \param Address The array index to start writing the serialized data to.
/// \param Data The data to serialize into the array.
template <typename T>
void Serialize(byte* Array, unsigned long Address, T Data)
{
    // Recast the data into a byte array.
    const byte* const Bytes = reinterpret_cast<const byte*>(&Data);
    // Copy the bytes into the data array.
    // Arduino is little endian.  Convert to big endian.
    unsigned int j = Address;
    for(unsigned int i = sizeof(Data); i > 0; i--)
    {
        Array[j++] = Bytes[(i-1)];
    }
    // Do not delete the byte array since it is just recasted from Data.
}
/// \brief Deserializes any data type from a specified array.
/// \param Array The array to deserialize the data from.
/// \param Address The array index to start reading the serialized data from.
/// \return The deserialized data.
template <typename T>
T Deserialize(const byte* Array, unsigned long Address)
{
    T Output;
    byte* Bytes = new byte[sizeof(Output)];
    // Arduino is little endian.  Convert to big endian.
    unsigned int j = Address;
    for(unsigned int i = sizeof(Output); i > 0; i--)
    {
        Bytes[i-1] = Array[j++];
    }
    Output = *(reinterpret_cast<const T*>(Bytes));
    delete [] Bytes;

    return Output;
}

}

#endif // SERIALIZATION_H
