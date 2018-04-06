#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING8BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/
#define TIMEOUT 20.0
int base;
int N;
int nextseqnum;
int nextacknum;
struct pkt sndPkt[1000];
struct pkt recvBufPkt[1000];
int sentPkt[1000];
int recvdPkt[1000];

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
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


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  printf("run A_output\n");
  struct pkt sendingpkt;

  strncpy(sendingpkt.payload, message.data, 20);
  sendingpkt.seqnum = nextseqnum;
  sendingpkt.checksum = calculate_checksum(sendingpkt);

  sndPkt[nextseqnum] = sendingpkt;

  printf("A sending: %s, seq: %d, base: %d\n",
          sndPkt[nextseqnum].payload, sndPkt[nextseqnum].seqnum, base);

  if(nextseqnum < base + N){

    tolayer3(0, sndPkt[nextseqnum]);

    if (nextseqnum == base){
      starttimer(0, TIMEOUT);
      printf("start timer\n" );
      // printf("cur time:%d\n", get_sim_time());
    }
  }
  nextseqnum += 1;
  printf("\n");

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  printf("run A_input\n");
  printf("A receving ack: %d, seqnum is: %d\n", packet.acknum, nextseqnum);

  if (packet.acknum > nextseqnum){
    return;
  }

  sentPkt[packet.acknum] = 1;

  if (base == packet.acknum){
    stoptimer(0);
    printf("cur time:%d\n", get_sim_time());
    base +=1;
    printf("stop timer\n");
    for (int i = 0; i < nextseqnum; i++){
      if (sentPkt[i] == 0){
        starttimer(0, TIMEOUT);
      }
    }
  }
  printf("\n");

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("run A_timerinterrupt\n");
  starttimer(0, TIMEOUT);
  printf("cur time:%d\n", get_sim_time());

  tolayer3(0, sndPkt[base]);
  printf("A resending: %s, seq: %d\n",  sndPkt[base].payload,  sndPkt[base].seqnum);
  printf("\n");

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  printf("run A_init\n");
  base = 0;
  N = getwinsize();
  nextseqnum = 0;
  for (int i = 0; i < N; i++){
    sentPkt[i] = 0;
  }

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt ackPkt;
  printf("run B_input\n");
  printf("B receving: %20s, seqnum: %d, current acknum: %d\n", packet.payload, packet.seqnum, nextacknum);

  //compare checksum
  int isCheckSumVaild = vaildiate_checksum(packet);

  if (isCheckSumVaild){
    //receving packet
    //check if deplicate pkt

    /*if the packet seq is equal to nextacknum, check if there is buffer,
    if have, send the buffer to upper layer, and send back the acknum to A,
    if have no buffer, just send the packet to layer5
    */
    if (packet.seqnum == nextacknum){
      tolayer5(1, packet.payload);
      //check if there is buffer
      for (int i = packet.seqnum - base; i < N; i++){
          if (recvBufPkt[i].acknum != -1){
            tolayer5(1, recvBufPkt[i].payload);
            recvBufPkt[i].acknum = -1;
            nextacknum += 1;
          }
      }
      //send ack packet
      ackPkt.acknum = packet.seqnum;
      printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
      nextacknum += 1;

    }else if (packet.seqnum > nextacknum){
      printf("B add buffer seq: %d\n", packet.seqnum);
      recvBufPkt[packet.seqnum - base] = packet;

      ackPkt.acknum = packet.seqnum;
      printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
    }else{
      //duplicate packet
      ackPkt.acknum = nextacknum - 1;
      printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
    }
  }
  printf("\n");

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  printf("run B_init\n");
  for (int i = 0; i < N; i++){
    recvBufPkt[i].acknum = -1;
  }

  for (int i = 0; i < 1000; i++){
    recvdPkt[i] = 0;
  }
}
