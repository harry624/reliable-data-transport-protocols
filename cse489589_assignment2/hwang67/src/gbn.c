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
#define TIMEOUT 20.0
int base;
int N;
int nextseqnum;
int nextacknum;
struct pkt sndpkt[1000];

int calculate_checksum(packet)
struct pkt packet;
{
  int checksum = 0;
  checksum += packet.seqnum;
  checksum += packet.acknum;

  int sizeOfArray = strlen(packet.payload);

  for(int i = 0; i < 20; i++){
    checksum += packet.payload[i];
  }
  //Perform bitwise inversion
  // checksum=~checksum;
  //Increment
  // checksum++;
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
  // //printf("run A_output\n");
  struct pkt sendingpkt;

  strncpy(sendingpkt.payload, message.data, 20);
  sendingpkt.seqnum = nextseqnum;
  sendingpkt.checksum = calculate_checksum(sendingpkt);
  sndpkt[nextseqnum] = sendingpkt;

  // //printf("A is sending: %s, seq: %d, base: %d\n", sndpkt[nextseqnum].payload, sndpkt[nextseqnum].seqnum, base);

  if(nextseqnum < base + N){

    // //printf("A sent: %s, seq: %d, base: %d\n", sndpkt[nextseqnum].payload, sndpkt[nextseqnum].seqnum, base);

    tolayer3(0, sndpkt[nextseqnum]);

    if (nextseqnum == base){
      starttimer(0, TIMEOUT);
    }
  }
  nextseqnum += 1;

  ////printf("\n");

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  // //printf("run A_input\n");
  // //printf("A receving ack: %d, next seqnum is: %d\n", packet.acknum, nextseqnum);

  if (packet.acknum < nextseqnum){
    base = packet.acknum + 1;
  }

  if (base == nextseqnum){
    stoptimer(0);
  }
  // else{
  //   starttimer(0, TIMEOUT);
  // }
  ////printf("\n");

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  // //printf("run A_timerinterrupt\n");
  starttimer(0, TIMEOUT);

  // //printf("%d\n", base + N);
  int max = (base + N) <= 1000 ? (base + N) : 1000;
  for (int i = base; i < max; i++){
    if(sndpkt[i].seqnum != -1){
      tolayer3(0, sndpkt[i]);
      // //printf("A resending :%s, seq: %d, base: %d\n", sndpkt[i].payload, sndpkt[i].seqnum, base);

    }
  }
  ////printf("\n");

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  // //printf("run A_init\n");
  base = 0;
  N = getwinsize();
  nextseqnum = 0;

  for (int i = 0; i < 1000; i++){
    sndpkt[i].seqnum = -1;
  }
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt ackPkt;
  //printf("run B_input\n");
  //printf("B receving: %s, seqnum: %d, current acknum: %d\n", packet.payload, packet.seqnum, nextacknum);

  //compare checksum
  int isCheckSumVaild = vaildiate_checksum(packet);

  if (isCheckSumVaild){
    //receving packet
    //check if deplicate pkt
    if (packet.seqnum == nextacknum){
      tolayer5(1, packet.payload);
      //send ack packet
      ackPkt.acknum = packet.seqnum;
      //printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
      nextacknum += 1;
    }else{
      //duplicate packet
      ackPkt.acknum = nextacknum - 1;
      // //printf("B sending ack: %d, packet discard\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
    }
  }
  ////printf("\n");

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  // //printf("run B_init\n");
  nextacknum = 0;
}
