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

//array to save the acknum of the pkt within the win_size
float timeoutArray[1000];

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
  timeoutArray[nextseqnum] = get_sim_time() + 20.0;
  printf("timeout sim time for pkt:%d is :%f\n",
            nextseqnum, timeoutArray[nextseqnum]);

  printf("A sending: %s, seq: %d, base: %d\n",
          sndPkt[nextseqnum].payload, sndPkt[nextseqnum].seqnum, base);

  if(nextseqnum < base + N){

    tolayer3(0, sndPkt[nextseqnum]);

    if (nextseqnum == base){
      starttimer(0, TIMEOUT);

      printf("start timer for seq: %d\n", nextseqnum);
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
  printf("sim time:%f\n", get_sim_time());

  printf("A receving ack: %d, nextseqnum is: %d, base: %d\n",
            packet.acknum, nextseqnum, base);

  if (packet.acknum > nextseqnum){
    return;
  }

  timeoutArray[packet.acknum] = 0.0;

  if (base == packet.acknum){
    stoptimer(0);
    printf("stop timer for seq: %d\n", base);
    base ++;

    for (int i = base; i < nextseqnum; i++){
      if (timeoutArray[i] > 0){
        starttimer(0, timeoutArray[i] - get_sim_time());
        break;
      }else if(timeoutArray[i] == 0.0){
        base = i + 1;
      }
    }
  }
  printf("\n");
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("run A_timerinterrupt\n");
  printf("sim time:%f\n", get_sim_time());
  starttimer(0, TIMEOUT);
  printf("start timer for seq: %d\n", base);

  tolayer3(0, sndPkt[base]);
  printf("A resending: %s, seq: %d\n",
          sndPkt[base].payload,  sndPkt[base].seqnum);
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
  printf("sim time:%f\n", get_sim_time());
  for (int i = 0; i < 1000; i++){
    timeoutArray[i] = -1.0;
  }

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  struct pkt ackPkt;
  printf("run B_input\n");
  printf("B receving: %s, seqnum: %d, current acknum: %d\n",
            packet.payload, packet.seqnum, nextacknum);

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
      printf("sending :%d to layer5: %s\n", packet.seqnum,  packet.payload);
      //check if there is buffer
      for (int i = packet.seqnum + 1; i < packet.seqnum + N && i < 1000; i++){
          if (recvBufPkt[i].acknum < 0){
            break;
          }
          if (recvBufPkt[i].acknum >= 0){
            printf("sending :%d to layer5: %s\n",i,  recvBufPkt[i].payload);
            tolayer5(1, recvBufPkt[i].payload);
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
      recvBufPkt[packet.seqnum] = packet;

      ackPkt.acknum = packet.seqnum;
      printf("B sending ack: %d\n", ackPkt.acknum);
      tolayer3(1, ackPkt);
    }else{
      //duplicate packet
      ackPkt.acknum = packet.seqnum;
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
  for (int i = 0; i < 1000; i++){
    recvBufPkt[i].acknum = -1;
  }

}
