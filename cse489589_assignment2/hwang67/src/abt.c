#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include "../include/queue.h"
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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
/*0 (for A-side delivery) or 1 (for B-side delivery) */
#define TIMEOUT 20.0
struct pkt sendBuf[1000]; //buffer for application layer sending msg
int pktsBufcnt;
int nextPktInd;
struct pkt tmpPkt;

int seqnum;
int acknum;
int lastchecksum;

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
  // checksum = ~checksum;

  // //Increment
  // checksum++;
  return checksum;
}

int vaildiate_checksum(packet)
struct pkt packet;
{
  int expectedChecksum = calculate_checksum(packet);
  // printf("expectedChecksum:%d, packetchecksum: %d\n", expectedChecksum, packet.checksum);
  return (expectedChecksum == packet.checksum);
}

//MAIN
/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  // printf("run A_output\n");
  struct pkt sendingpkt;

  strncpy(sendingpkt.payload, message.data, 20);
  sendingpkt.seqnum = seqnum;
  sendingpkt.checksum = calculate_checksum(sendingpkt);

  seqnum = sendingpkt.seqnum == 0 ? 1: 0;

  //send the packet
  pktsBufcnt++;
  sendBuf[pktsBufcnt] = sendingpkt;
  // printf("A is sending : %20s, seq: %d, nextindex: %d\n",
  //         sendBuf[pktsBufcnt].payload, sendBuf[pktsBufcnt].seqnum, nextPktInd);

  if(nextPktInd == pktsBufcnt){
  tolayer3(0, sendBuf[nextPktInd]);
  // printf("A sent : %20s, seq: %d\n",
  //         sendBuf[nextPktInd].payload, sendBuf[nextPktInd].seqnum);
  starttimer(0, TIMEOUT);
  }

  //printf("\n");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  // printf("run A_input\n");
  // printf("A receving ack: %d, nextseqnum: %d, bufcnt: %d\n", packet.acknum, nextPktInd, pktsBufcnt);

  if (packet.acknum == sendBuf[nextPktInd].seqnum){
    nextPktInd++;
    stoptimer(0);

    if (nextPktInd <= pktsBufcnt){
      tolayer3(0, sendBuf[nextPktInd]);
      starttimer(0, TIMEOUT);
    }
  }
  //printf("\n");

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  // printf("run A_timerinterrupt\n");
  // printf("A resending : %20s, seq: %d, nextindex: %d\n",
          // sendBuf[nextPktInd].payload, sendBuf[nextPktInd].seqnum, nextPktInd);
  tolayer3(0, sendBuf[nextPktInd]);
  starttimer(0, TIMEOUT);
  //printf("\n");

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  // printf("run A_init\n");
  // memest(&tmp,0 ,sizeof(struct pkt));
  seqnum = 0;
  pktsBufcnt = -1;
  nextPktInd = 0 ;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt ackPkt;
  // printf("run B_input\n");
  // printf("B receving: %20s, seqnum: %d, current acknum: %d\n", packet.payload, packet.seqnum, acknum);

  //compare checksum
  int isCheckSumVaild = vaildiate_checksum(packet);

  if (isCheckSumVaild){
    //receving packet
    //check if deplicate pkt
    if (packet.seqnum == acknum){
      tolayer5(1, packet.payload);
      //send ack packet
      ackPkt.acknum = packet.seqnum;
      // printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
      acknum = acknum == 0 ? 1 : 0;
    }else{
      //duplicate packet
      ackPkt.acknum = packet.seqnum;
      // printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
      // acknum = acknum == 0 ? 1 : 0;
    }
  }
  //printf("\n");

}

/* the following routine will be called once (only) before any other */
/* entity B routines are called-> You can use it to do any initialization */
void B_init()
{
  // printf("run B_init\n");
  acknum = 0;
}
