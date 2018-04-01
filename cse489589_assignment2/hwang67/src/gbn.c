#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
int base;
int win_size;
int seqnum;
int acknum;

int calculate_checksum(packet)
struct pkt packet;
{
  int checksum = 0;
  checksum += packet.seqnum;
  checksum += packet.acknum;
  for(int i = 0; i < sizeof(packet.payload) / sizeof(char); i++){
    checksum += packet.payload[i];
  }
  return checksum;
}

int vaildiate_checksum(packet)
struct pkt packet;
{
  int expectedChecksum = calculate_checksum(packet);
  return (expectedChecksum == packet.checksum);
}

void A_output(message)
  struct msg message;
{
  // printf("run A_output\n");
  // printf("run A_output\n");
  struct pkt sendingpkt;

  strncpy(sendingpkt.payload, message.data, 20);
  sendingpkt.seqnum = seqnum;
  sendingpkt.checksum = calculate_checksum(sendingpkt);

  tolayer3(0, sendingpkt);
  starttimer(0, TIMEOUT);
  seqnum += 1;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  // printf("run A_input\n");
  if (packet.acknum == seqnum - win_size){
      seqnum += 1;
      stoptimer(0);
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  // printf("run A_timerinterrupt\n");

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  // printf("run A_init\n");
  base = 0;
  win_size = getwinsize();
  seqnum = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  // printf("run B_input\n");
  //deplicate pkt
  if (packet.seqnum != acknum){
    memset(&packet.payload[0], 0, sizeof(packet.payload));
    packet.acknum = acknum;
    //send ack packet
    tolayer3(1, packet);
  }

  //compare checksum
  int isCheckSumVaild = vaildiate_checksum(packet);
  if (isCheckSumVaild){
    // printf("run B_input\n");
    //receving packet
    tolayer5(1, packet.payload);

    memset(&packet.payload[0], 0, sizeof(packet.payload));
    packet.acknum = packet.seqnum;
    //send ack packet
    tolayer3(1, packet);
    acknum += 1;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  // printf("run B_init\n");
  win_size = getwinsize();
  acknum = 0;
}
