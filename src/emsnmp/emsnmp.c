/*
 *  Embedded SNMP Agent Main Function and Parsing Functions
 *
 *  ./software/ch8/emsnmp/emsnmp.c
 *
 *  M. Tim Jones <mtj@cogitollc.com>
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include "emsnmp.h"
#include "snmpdata.h"
#include "user.h"
#include <hhp_error_code.h>
#include <hhp.h>

extern struct msg_entry msg_data[];
extern const int max_entries;

struct messageStruct {
  unsigned char buffer[512];
  int  len;
  int  index;
};

/* PDU Message Var */
struct pdu_var {
  enum hh_oid_name   name;
  enum hh_oid_type   type;
  int                index;
  void              *val;
  uint32_t           val_len;
};

/* Current PDU Message Vars list */
static struct pdu_var pdu_vars[10];

/* Current PDU Message Var position */
static int pdu_vars_pos;
/* Current PDU Message Var number */
static int pdu_vars_num;

static struct messageStruct request, response;

unsigned char errorStatus, errorIndex;


typedef struct {
  int start;	/* Absolute Index of the TLV */
  int len;	/* The L value of the TLV */
  int vstart;   /* Absolute Index of this TLV's Value */
  int nstart;   /* Absolute Index of the next TLV */
} tlvStructType;

void hh_msg_get_send_info(uint8_t **data, uint32_t *len)
{
  *data = request.buffer;
  *len = request.len;
}
static int parseLength(const unsigned char *msg, int *len)
{
  int i=1;

  if (msg[0] & 0x80) {
    int tlen = (msg[0] & 0x7f) - 1;
    *len = msg[i++];

    while (tlen--) {
      *len <<= 8;
      *len |= msg[i++];
    }

  } else {
    *len = msg[0];
  }

  return i;
}


static int parseTLV(const unsigned char *msg, int index, tlvStructType *tlv)
{
  int Llen = 0;

  tlv->start = index;

  Llen = parseLength((const char *)&msg[index+1], &tlv->len );

  tlv->vstart = index + Llen + 1;

  switch (msg[index]) {
    case SEQUENCE:
    case GET_REQUEST:
    case GET_NEXT_REQUEST:
    case SET_REQUEST:
      tlv->nstart = tlv->vstart;
      break;
    default:
      tlv->nstart = tlv->vstart + tlv->len;
      break;
  }

  return 0;
}


#define COPY_SEGMENT(x) \
	{ \
		request.index += seglen; \
    		memcpy(  &response.buffer[response.index], \
		         &request.buffer[x.start], seglen ); \
	        response.index += seglen; \
	}


/*
 * Given a TLV and a size, this inserts the size argument into the L element
 * of the response TLV.
 */
static void insertRespLen(int reqStart, int respStart, int size)
{
  int indexStart, lenLength;
  unsigned int mask = 0xff;
  int shift = 0;

  if (request.buffer[reqStart+1] & 0x80) {
    lenLength = request.buffer[reqStart+1] & 0x7f;
    indexStart = respStart+2;

    while (lenLength--) {
      response.buffer[indexStart+lenLength] =
            (unsigned char)((size & mask) >> shift);
      shift+=8;
      mask <<= shift;
    }

  } else {
    response.buffer[respStart+1] = (unsigned char)(size & 0xff);
  }
}

static int parseSequence ( int reqType, int index )
{
  int ret = -1, seglen;
  tlvStructType seq;
  int size = 0, respLoc;

  ret = parseTLV(request.buffer, request.index, &seq);

  if ( request.buffer[seq.start] != SEQUENCE ) return -1;

  seglen = seq.vstart - seq.start;
  respLoc = response.index;
  COPY_SEGMENT(seq);

  return size;
}


static int parseSequenceOf ( int reqType )
{
  int ret = -1, seglen;
  tlvStructType seqof;
  int size = 0, respLoc;
  int index = 0;

  ret = parseTLV(request.buffer, request.index, &seqof);

  if ( request.buffer[seqof.start] != SEQUENCE_OF ) return -1;

  seglen = seqof.vstart - seqof.start;
  respLoc = response.index;
  COPY_SEGMENT(seqof);

  while (request.index < request.len) {
    size += parseSequence( reqType, index++ );
  }

  insertRespLen(seqof.start, respLoc, size);

  return size;
}


static int parseRequest ( )
{
  int ret = -1, seglen;
  tlvStructType snmpreq, requestid, errStatus, errIndex;
  int size = 0, respLoc, reqType;

  ret = parseTLV(request.buffer, request.index, &snmpreq);

  reqType = request.buffer[snmpreq.start];

  if ( !VALID_REQUEST(reqType) ) return -1;

  seglen = snmpreq.vstart - snmpreq.start;
  respLoc = snmpreq.start;
  size += seglen;
  COPY_SEGMENT(snmpreq);

  response.buffer[snmpreq.start] = GET_RESPONSE;

  parseTLV(request.buffer, request.index, &requestid);
  seglen = requestid.nstart - requestid.start;
  size += seglen;
  COPY_SEGMENT(requestid);

  parseTLV(request.buffer, request.index, &errStatus);
  seglen = errStatus.nstart - errStatus.start;
  size += seglen;
  COPY_SEGMENT(errStatus);

  parseTLV(request.buffer, request.index, &errIndex);
  seglen = errIndex.nstart - errIndex.start;
  size += seglen;
  COPY_SEGMENT(errIndex);

  ret = parseSequenceOf(reqType);
  if (ret == -1) return -1;
  else size += ret;

  insertRespLen(snmpreq.start, respLoc, size);

  /* Store the error status and index, in the event an error was found */
  if (errorStatus) {
    response.buffer[errStatus.vstart] = errorStatus;
    response.buffer[errIndex.vstart] = errorIndex + 1;
  }

  return size;
}


static int parseCommunity ( )
{
  int ret = -1, seglen;
  tlvStructType community;
  int size=0;

  ret = parseTLV(request.buffer, request.index, &community);

  if (!((request.buffer[community.start] == OCTET_STRING) &&
        (community.len == COMMUNITY_SIZE))) return -1;

  if (!memcmp(&request.buffer[community.vstart],
             (char *)COMMUNITY, COMMUNITY_SIZE)) {

    seglen = community.nstart - community.start;
    size += seglen;
    COPY_SEGMENT(community);

    size += parseRequest();

  } else {
    return -1;
  }

  return size;
}


static int parseVersion ( )
{
  int size = 0, seglen;
  tlvStructType tlv;

  size = parseTLV(request.buffer, request.index, &tlv);

  if (!((request.buffer[tlv.start] == INTEGER) &&
        (request.buffer[tlv.vstart] == SNMP_V1))) return -1;

  seglen = tlv.nstart - tlv.start;
  size += seglen;
  COPY_SEGMENT(tlv);
  size = parseCommunity();

  if (size == -1) return size;
  else return (size + seglen);
}


static int parseSNMPMessage ( )
{
  int size = 0, seglen, ret, respLoc;
  tlvStructType tlv;

  ret = parseTLV(request.buffer, request.index, &tlv);

  if (request.buffer[tlv.start] != SEQUENCE_OF) return -1;

  seglen = tlv.vstart - tlv.start;
  respLoc = tlv.start;
  COPY_SEGMENT(tlv);

  size = parseVersion();

  if (size == -1) return -1;
  else size += seglen;

  insertRespLen(tlv.start, respLoc, size);

  return ret;
}

void snmp_incoming_msg(unsigned char *data, unsigned int len)
{
  if (data == NULL) {
    response.index = 0;
    response.len = 0;
    return;
  }
  memcpy(response.buffer + response.len, data, len);
  response.len += len;
}

unsigned char snmp_parse_type()
{
  tlvStructType tlv;
  unsigned char pdu_type;
  bool matched = false;
  int i = 0;
  int next_sequence;

  // TODO Process SNMP package
  pdu_vars_pos = 0;
  pdu_vars_num = 0;

  /* Firstly we need check if this package is gateway information query
     reponse messasge */
  if (response.len > 20) {
    if (memcmp("Gateway,Resp,", response.buffer, 13) == 0) {
      return HH_MSG_GATEWAY;
    }
  }
  /**/
  parseTLV(response.buffer, response.index, &tlv);
  if (response.buffer[tlv.start] != SEQUENCE_OF) {
    goto err_msg;
  }

  parseTLV(response.buffer, tlv.vstart, &tlv);

  /* V1 or v2 operation */
  if (response.buffer[tlv.start] != INTEGER) {
    goto err_msg;
  }

  /* Ignore compare Communication Key, looks no sense in embedded device */
  parseTLV(response.buffer, tlv.nstart, &tlv);
  if (response.buffer[tlv.start] != OCTET_STRING)
  {
    goto err_msg;
  }

  /* SNMP Package Type */
  if ((response.buffer[tlv.nstart] != GET_RESPONSE) &&
      (response.buffer[tlv.nstart] != SET_REQUEST) &&
      (response.buffer[tlv.nstart] != TRAP_V1) &&
      (response.buffer[tlv.nstart] != TRAP_V2) &&
      (response.buffer[tlv.nstart] != TRAP_INFORM)) {
	goto err_msg;
  }

  pdu_type = response.buffer[tlv.nstart];

  /* Parse SEQUENCE, next parseTLV should be start from tlv.vstart */
  parseTLV(response.buffer, tlv.nstart, &tlv);

  /* Parse Response */
  /* Request ID */
  parseTLV(response.buffer, tlv.vstart, &tlv);
  /* Error status */
  parseTLV(response.buffer, tlv.nstart, &tlv);
  /* Error index */
  parseTLV(response.buffer, tlv.nstart, &tlv);

  /* Parse SEQUENCE(Variable Binding,
     next parseTLV should be start from tlv.vstart,
     and should be SEQUENCE too */
  parseTLV(response.buffer, tlv.nstart, &tlv);
  if (response.buffer[tlv.start] != SEQUENCE_OF) {
    goto err_msg;
  }

  matched = false;
  /* The index should be point to the first OID sequence */
  response.index = tlv.vstart;
  while (response.index < response.len) {
    /* Should be SEQUENCE */
    parseTLV(response.buffer, response.index, &tlv);
    if (response.buffer[tlv.start] != SEQUENCE_OF) {
      goto err_msg;
    }
    /* OBJECT IDENTIFIER */
    parseTLV(response.buffer, tlv.vstart, &tlv);
    if (response.buffer[tlv.start] != OBJECT_IDENTIFIER) {
      goto err_msg;
    }

    for(i = 0; i < max_entries; i++) {
      if (tlv.len <= msg_data[i].oid_len) {
        if (memcmp(response.buffer + tlv.vstart, msg_data[i].oid, msg_data[i].oid_len) == 0) {
          /* Found valid entry and break loop, i will point to the entry */
          matched = true;
          break;
        }
      }
    }
    if (matched == false) {
      // printf unknow OID;
      parseTLV(response.buffer, tlv.nstart, &tlv);
      response.index = tlv.nstart;
      continue;
    }
    pdu_vars[pdu_vars_num].name = msg_data[i].oid_name;
    pdu_vars[pdu_vars_num].type = msg_data[i].oid_type;
    pdu_vars[pdu_vars_num].index = msg_data[i].index;
    /* Value TLV */
    parseTLV(response.buffer, tlv.nstart, &tlv);
    response.index = tlv.nstart;

    if (response.buffer[tlv.start] == INTEGER) {
      pdu_vars[pdu_vars_num].val = response.buffer + tlv.vstart;
      pdu_vars[pdu_vars_num].val_len = tlv.len;
    }
    pdu_vars_num++;
  }
  response.len = 0;
  response.index = 0;
  return pdu_type;

err_msg:
  response.len = 0;
  response.index = 0;
  pdu_vars_pos = 0;
  pdu_vars_num = 0;
  return -1;
}

static inline int32_t hh_msg_get_int_val(uint8_t *val, uint32_t len)
{
  if (len == 1) {
    return *val;
  } else if (len == 2) {
    return val[0] << 8 | val[1];
  } else if (len == 4) {
    return (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | (val[3]);
  }
  return 0;
}

int32_t hh_msg_get_next_item(struct hh_oid_entry *entry)
{
  if (pdu_vars_pos < pdu_vars_num) {
    entry->oid_name = pdu_vars[pdu_vars_pos].name;
    entry->oid_type  = pdu_vars[pdu_vars_pos].type;
    entry->index  = pdu_vars[pdu_vars_pos].index;
    if (pdu_vars[pdu_vars_pos].type == INTEGER)
    entry->val.int_val = hh_msg_get_int_val((uint8_t *)(pdu_vars[pdu_vars_pos].val), pdu_vars[pdu_vars_pos].val_len);
    pdu_vars_pos++;
  }
  else {
    return HHP_NO_RESOURCE;
  }
  return HHP_STATUS_OK;
}

int32_t hh_msg_set_item(struct hh_oid_entry *entry, uint8_t size)
{
  uint8_t setinfo[27] = {
    0x30, 0x2C,
    0x02, 0x01, 0x01, 0x04, 0x07, 0x70, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65,
    0xA3, 0x1E,
    0x02, 0x01, 0x0B, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
    0x30, 0x00
  };
  int i = 0;
  int j = 0;

  memcpy(request.buffer, setinfo, 27);
  request.index = 27;
  request.len = 27;

  for (i = 0; i < size; i++) {
    for(j = 0; j < max_entries; j++) {
      if ((entry[i].oid_name == msg_data[j].oid_name) &&
          (entry[i].index == msg_data[j].index)
          )
      {
        request.buffer[request.len++] = SEQUENCE;
        request.buffer[request.len++] = 0x00;
        request.buffer[request.len++] = OBJECT_IDENTIFIER;
        request.buffer[request.len++] = msg_data[j].set_len;
        request.index += 0x01; /* Point to length, need to update after append value */



        memcpy(request.buffer + request.len, msg_data[j].oid, msg_data[j].set_len);
        request.len += msg_data[j].set_len;
        request.buffer[request.index] = 0x02 + msg_data[j].set_len;

        if (msg_data[j].oid_type == HH_OID_INTEGER) {
          request.buffer[request.len++] = INTEGER;
          if (entry[i].val.int_val <= 0xFF) {
            request.buffer[request.len++] = 0x01;
            request.buffer[request.len++] = entry[i].val.int_val & 0xFF;
            request.buffer[request.index] += 3;
          } else {
            request.buffer[request.len++] = 0x04;
            request.buffer[request.len++] = entry[i].val.int_val >> 24;
            request.buffer[request.len++] = (entry[i].val.int_val >> 16) & 0xFF;
            request.buffer[request.len++] = (entry[i].val.int_val >> 8) & 0xFF;
            request.buffer[request.len++] = entry[i].val.int_val & 0xFF;
            request.buffer[request.index] += 6;
            }
        }
        request.index = request.len;
        break;
      }
    }
  }
  /* Update other length variable */
  request.buffer[26] = request.len - 27;
  request.buffer[15] = request.len - 16;
  request.buffer[1] = request.len - 2;

  HHP_DEBUG_HEXSTRING(i, request.buffer, request.len);
  //for (i = 0; i < request.len; i++) {
  //  printf ("%02x ", request.buffer[i]);
  //}
  //printf("\n");
  return HHP_STATUS_OK;
}

int32_t hh_msg_set_item_raw(uint8_t *oid, uint8_t oid_len, uint32_t value)
{
  uint32_t i;
  uint8_t setinfo[27] = {
    0x30, 0x2C,
    0x02, 0x01, 0x01, 0x04, 0x07, 0x70, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65,
    0xA3, 0x1E,
    0x02, 0x01, 0x0B, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
    0x30, 0x00
  };

  memcpy(request.buffer, setinfo, 27);
  request.index = 27;
  request.len = 27;

  request.buffer[request.len++] = SEQUENCE;
  request.buffer[request.len++] = 0x00;
  request.buffer[request.len++] = OBJECT_IDENTIFIER;
  request.buffer[request.len++] = oid_len;
  request.index += 0x01; /* Point to length, need to update after append value */

  memcpy(request.buffer + request.len, oid, oid_len);
  request.len += oid_len;
  request.buffer[request.index] = 0x02 + oid_len;

  /* Process Value */
  request.buffer[request.len++] = INTEGER;
  if (value <= 0xFF) {
    request.buffer[request.len++] = 0x01;
    request.buffer[request.len++] = value & 0xFF;
    request.buffer[request.index] += 3;
  } else {
    request.buffer[request.len++] = 0x04;
    request.buffer[request.len++] = value >> 24;
    request.buffer[request.len++] = (value >> 16) & 0xFF;
    request.buffer[request.len++] = (value >> 8) & 0xFF;
    request.buffer[request.len++] = value & 0xFF;
    request.buffer[request.index] += 6;
  }

  /* Update other length variable */
  request.buffer[26] = request.len - 27;
  request.buffer[15] = request.len - 16;
  request.buffer[1] = request.len - 2;

  HHP_DEBUG_HEXSTRING(i, request.buffer, request.len);

  return HHP_STATUS_OK;
}

int32_t hh_msg_get_item(struct hh_oid_entry *entry, uint8_t size)
{
  uint8_t setinfo[27] = {
    SEQUENCE, 0xFF,
    INTEGER, 0x01, 0x01, OCTET_STRING, 0x06, 0x70, 0x75, 0x62, 0x6C, 0x69, 0x63,
    GET_REQUEST, 0xFF,
    INTEGER, 0x01, 0x00, INTEGER, 0x01, 0x00, INTEGER, 0x01, 0x00,
    SEQUENCE, 0xFF
  };
  int i = 0;
  int j = 0;

  memcpy(request.buffer, setinfo, 26);
  request.index = 26;
  request.len = 26;

  for (i = 0; i < size; i++) {
    for(j = 0; j < max_entries; j++) {
      if ((entry[i].oid_name == msg_data[j].oid_name) &&
          (entry[i].index == msg_data[j].index)
          )
      {
        request.buffer[request.len++] = SEQUENCE;
        request.buffer[request.len++] = 0x00;
        request.buffer[request.len++] = OBJECT_IDENTIFIER;
        request.buffer[request.len++] = msg_data[j].set_len;
        request.index += 0x01; /* Point to length, need to update after append value */

        memcpy(request.buffer + request.len, msg_data[j].oid, msg_data[j].set_len);
        request.len += msg_data[j].set_len;
        request.buffer[request.index] = 0x02 + msg_data[j].set_len;

        /* Append Null Value */
        request.buffer[request.len++] = NULL_ITEM;
        request.buffer[request.len++] = 0x00;
        request.buffer[request.index] += 2;
        request.index = request.len;
        break;
      }
    }
  }
  /* Update other length variable */
  request.buffer[25] = request.len - 26;
  request.buffer[14] = request.len - 15;
  request.buffer[1] = request.len - 2;

  HHP_DEBUG_HEXSTRING(i, request.buffer, request.len);
  return HHP_STATUS_OK;
}

void hh_msg_resp_buffer(uint8_t **data, uint32_t *len)
{
    *data = (uint8_t *)response.buffer;
    *len = response.len;
}
void hh_msg_gateway_query_detail(void)
{
    memcpy(request.buffer, "Gateway,Query,Detail;", 21);
    request.len = 21;
}
void hh_msg_gateway_query_devlist(void)
{
    memcpy(request.buffer, "Gateway,Query,DevList;", 22);
    request.len = 22;
}
void hh_msg_gateway_query_taglist(void)
{
    memcpy(request.buffer, "Gateway,Query,TagList;", 22);
    request.len = 22;
}
/*---------------------------------------------------------------------------
 * main() - The embedded SNMP server main
 *-------------------------------------------------------------------------*/
#if 0
int main(int argc, char *argv[])
{
  int snmpfd;
  struct sockaddr_in servaddr;
  struct sockaddr from;
  int fromlen;
  int retStatus;

  extern void initTable( void );

  initTable();

  snmpfd = socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(161);

  retStatus = bind(snmpfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (retStatus < 0) {
    printf("Unable to bind to socket (%d).\n", errno);
    exit(-1);
  }

  printf("Started the SNMP agent\n");

  for ( ; ; ) {

    fromlen = sizeof(from);
    request.len = recvfrom(snmpfd,
                            &request.buffer[0], 1024, 0, &from, &fromlen);

    printf("Received datagram from %s, port %d\n",
           inet_ntoa(((struct sockaddr_in *)&from)->sin_addr),
           ((struct sockaddr_in *)&from)->sin_port);

    if (request.len >= 0) {

#if 1
      {
        int i;
        printf("\nRequest: \n");
        for (i = 0 ; i < request.len ; i++) {
          if ((i % 16) == 0) printf("\n  %04x : ", i);
          printf("%02x ", (unsigned char)request.buffer[i]);
        }
        printf("\n\n");
      }
#endif

      request.index = 0;
      response.index = 0;
      errorStatus = errorIndex = 0;

      if (parseSNMPMessage() != -1) {

        sendto(snmpfd, response.buffer, response.index, 0,
                (struct sockaddr *)&from, fromlen);

      }

#if 1
      {
        int i;
        printf("\nResponse: \n");
        for (i = 0 ; i < response.index ; i++) {
          if ((i % 16) == 0) printf("\n  %04x : ", i);
          printf("%02x ", (unsigned char)response.buffer[i]);
        }
        printf("\n\n");
      }
#endif

    }

  }

  close(snmpfd);
  return(0);
}
#endif

/*
 *  Copyright (c) 2002 Charles River Media.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or
 *  without modification, is hereby granted without fee provided
 *  that the following conditions are met:
 *
 *    1.  Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *    2.  Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *    3.  Neither the name of Charles River Media nor the names of
 *        its contributors may be used to endorse or promote products
 *        derived from this software without specific prior
 *        written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHARLES RIVER MEDIA AND CONTRIBUTERS
 * 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CHARLES
 * RIVER MEDIA OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARAY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

