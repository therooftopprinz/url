# UDP Reliability Layer

UDP Reliability Layer is a user protocol that will provide connectionless reliable packet transmission.

**URL Format**

URL is composed of mutiple headers and might contain a payload.

| |
|:--:|
| Header 0 |
| Header .. |
| Header n |
| ... |
| Payload |
| |

## Header Formats

**Acknowledgement Header**

**ACK** (Or **NACK** If **URL Protocol Information : NACK Information**) of the arrived message. Must not have payload.

| \[0-3\](31:30) | \[0-3\](29:0) | \[4-7\](31:16) | \[4-7\](15:0) |
|:--------------:|:-------------:|:--------------:|:-------------:|
| HT = 00        | OFFSET        | MSG_ID         | MAC           |

* HT (2-bit)- Header Type.
* OFFSET (30-bit) - URL Message Offset for the corresponding ACK/NACK.
* MSG_ID (16-bit) - Message Identifier for the corresponding ACK/NACK.
* MAC (16-bit) - Message Authentication Code.

**URL Message Initial Data Header**

Indicates the start of trasmission of URL Message. Contains the size of URL Message to be sent. Payload is is required. Payload OFFSET is 0. Last header before payload.

| \[0-3\](31:30) | \[0-3\](29:0) | \[4-7\](31:16) | \[4-7\](15:0) | \[8-11\](31:0) |
|:--------------:|:-------------:|:--------------:|:-------------:|:--------------:|
| HT = 01        |  SZ           | MSG_ID         | MAC           | SEND_OPT       |

* HT (2-bit) - Header.
* SZ (30-bit) - URL Message size.
* MSG_ID (16-bit) - Message Identifier for the corresponding segment payload.
* MAC (16-bit)- Message Authentication Code
* SEND_OPT - Send option:
  * 1:0 - Security Type.
  	* 00 - No security
  	* 01 - Integrity protected
  	* 10 - Integrity and ciphered
  	* 11 - Integrity and ciphered with new context.
  * 8 - Acknowledge Mode.

**URL Message Data Header**

Indicates URL Segment Message. Payload is required. Last header before payload.

| \[0-3\](31:30) | \[0-3\](29:0) | \[4-7\](31:16) | \[4-7\](15:0) |
|:--------------:|:-------------:|:--------------:|:-------------:|
| HT = 10        | OFFSET        | MSG_ID         | MAC           |

* HT (2-bit) - Header Type.
* OFSSET (30-bit) - URL Message Offset for the corresponding segment payload.
* MSG_ID (16-bit) - Message Identifier for the corresponding segment payload.
* MAC (16-bit) - Message Authentication Code.

**URL Protocol Information : NACK Information**

Indicated that there is an error in the received URL Message Segment. When present, next header is always Acknowledgement Header. Last header.

| \[0-3\](31:30) | \[0-3\](29:27) | \[0-3\](26:0) |
|:--------------:|:-------------:|:--------------:|
| HT = 11        | ID=000        | INFO           |

* HT (2-bit) - Header Type.
* ID (3-bit) - Infomation type.
* INFO (27-bit) - Information Label:
  * INFO == 1 - Integrity Protection Failed.
  * INFO == 2 - Decryption Failed.
  * INFO == 3 - Service Denied.

## Operation
* When an URL message is requested to be the message size is larger than the configured URL MTU, the message will be segmented into several transmission units.
* URL Message Segment transmission size varies from channel quality and is determined during the transmission.
* In the sender the segments are transmitted sequentially every URL MESSAGE SEGMENT TX INTERVAL.
* URL Message Segment 0 is sent first, the subsequent URL Message Segments will not be sent if the URL Message Segment is ACKed.
* Every sent segment must have an ACK from receiver within URL SEGMENT TIMEOUT window.
* If the URL MESSAGE SEGMENT TIMEOUT expires without an ACK, failed segment will be transmitted again.
* Each retransmission decreases the URL Segment transmission size and increases the URL MESSAGE SEGMENT TX INTERVAL and URL MESSAGE SEGMENT TIMEOUT. 
* If the URL RETX failed URL MAX URL MSG RETX times, send function will return with an URL MESSAGE TX ERROR.

## URL Message Segment Size Selection
## URL Message Segment Transmit Inverval Selection
## URL Message Segment Timeout Selection
