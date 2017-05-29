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

**ACK** (Or **NACK** If NACK **URL Protocol Information : NACK Information**) of the arrived message. No Payload. Must not contain with **URL Message Data Header**.


| \[0-3\](31:30) | \[0-3\](29:0) | \[4-7\](31:0) |
|:--------------:|:-------------:|:-------------:|
| HT = 00        | OFSSET        | MSG_ID        |


* HT (2-bit)- Header Type
* SQN (30-bit) - URL Message Offset for the corresponding ACK/NACK
* MSG_ID - Message Identifier for the corresponding ACK/NACK

**URL Message Size Header**

Indicates the size of URL message that will be sent. This message is only present when URL Message data Header is present and has OFFSET=0.

| \[0-3\](31:30) | \[0-3\](29:0) |
|:--------------:|:-------------:|
| HT = 01        |  SZ           |

* HT (2-bit) - Header Type
* SZ (30-bit) - URL Message size

**URL Message Data Header**

Always last header when available. It specifies the segment offset of the URL message.

| \[0-3\](31:30) | \[0-3\](29:0) | \[4-7\](31:0) |
|:--------------:|:-------------:|:-------------:|
| HT = 10        | OFFSET        | MSG_ID        |

* HT (2-bit) - Header Type
* OFSSET (30-bit) - URL Message Offset for the corresponding segment payload.
* MSG_ID - Message Identifier for the corresponding segment payload.

**URL Protocol Information : NACK Information**

Indicated that there error in the received URL Message. When present, next header is always Acknowledgement Header.

| \[0-3\](31:30) | \[0-3\](29:27) | \[4-7\](26:0) |
|:--------------:|:-------------:|:--------------:|
| HT = 11        | ID=0          | INFO           |

* HT (2-bit) - Header Type
* ID (3-bit) - Infomation type
* INFO (27-bit) - Information Label
  * INFO = 0 : RE TX DATA MISMATCH.


**URL Protocol Information : Sender Request**

Indicates that the sender has a request to the receiver

| \[0-3\](31:30) | \[0-3\](29:27) | \[4-7\](26:0) |
|:--------------:|:-------------:|:--------------:|
| HT = 11        | ID=1          | INFO           |

* HT (2-bit) - Header Type
* ID (3-bit) - Infomation type
* INFO (27-bit) - Information Label
  * INFO = 0 : RE URL MSG RE TX. Tell receiver that the send will retransmit the URL MESSAGE from the start. Then sender will not transmit the subsequent URL SEGMENT if this message is not ACKed.


## Operation
When an URL message is requested to be the message size is larger than the configured URL MTU, the message will be segmented into several transmission units. URL MTU varies from channel quality and is determined during the length of the transmission. In the sender the segments are transmitted sequentially every URL SEGMENT TX INTERVAL. Every sent segment must have an ACK from receiver within URL SEGMENT TIMEOUT window, if the timeout expires without an ACK, failed segment will be transmitted again. Each retransmission decreases the URL MTU and increases the URL SEGMENT TX INTERVAL. If the total segment retransmission is equal to URL MAX SEGMENT RE TX, the URL MESSAGE is resent from offset 0 with and with a URL Protocol Information: Sender Request INFO = RE URL MSG RE TX. In URL RE TX the sender will not send the subsequent URL SEGMENT if the first SEGMENT is ACKed. If the URL RE TX failed URL MAX URL MSG RE TX times, send function will return with an URL MESSAGE TX ERROR. 
