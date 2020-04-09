#include <stdio.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "feature.pb.h"

//nanopb also supports static allocation for strings, e.g. required string name = 1 [(nanopb).max_size = 40];
//this is to illustrate how callback is used for variable length field
bool write_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    char *str = "crawler";
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **buf)
{
    return pb_read(stream, (pb_byte_t *) *buf, stream->bytes_left);
}


int main()
{
    /* This is the buffer where we will store our message. */
    uint8_t buffer[128];
    size_t message_length;
    bool status;
    
    /* Encode our message */
    {
        /* Allocate space on the stack to store the message data.
         *
         * Nanopb generates simple struct definitions for all the messages.
         * - check out the contents of simple.pb.h!
         * It is a good idea to always initialize your structures
         * so that you do not have garbage data from RAM in there.
         */
        loupepb_feature_FeaturesMessages message = loupepb_feature_FeaturesMessages_init_zero;
        
        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        
        /* Fill in the lucky number */
        message.company_id = 13;
        message.has_company_id = 1;

        //nanopb also supports static allocation, e.g. required string name = 1 [(nanopb).max_size = 40];
        //message.has_collector_guid = 1;
        //strcpy(message.collector_guid, "crawler");

        message.collector_guid.funcs.encode = &write_string;
        
        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, loupepb_feature_FeaturesMessages_fields, &message);
        message_length = stream.bytes_written;
        
        /* Then just check for any errors.. */
        if (!status)
        {
            printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }
    }
    
    /* Now we could transmit the message over network, store it in a file or
     * wrap it to a pigeon's leg.
     */

    /* But because we are lazy, we will just decode it immediately. */
    
    {
        /* Allocate space for the decoded message. */
        loupepb_feature_FeaturesMessages message = loupepb_feature_FeaturesMessages_init_zero;
        
        /* Create a stream that reads from the buffer. */
        pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);
        
        /* Now we are ready to decode the message. */
        char collector_guid[128];
        message.collector_guid.funcs.decode = &decode_string;
        message.collector_guid.arg = collector_guid;
        status = pb_decode(&stream, loupepb_feature_FeaturesMessages_fields, &message);
        
        /* Check for errors... */
        if (!status)
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }
        
        /* Print the data contained in the message. */
        printf("company_id=%llu, crawler_id=%s\n", message.company_id, collector_guid);
    }
    
    return 0;
}

