// TODO:临时使用，以后将本模块删除

#include "rtthread.h"

#define MESSGES_BEGIN 0x5AA5u
#define MESSGES_END 0x6996u

#define HTONS(n) (uint16_t)((((n)&0xFF) << 8) | (((n)&0xFF00) >> 8))
#define HTONL(n)                                        \
  (uint32_t)((((n)&0xFF) << 24) | (((n)&0xFF00) << 8) | \
             (((n)&0xFF0000UL) >> 8) | (((n)&0xFF000000UL) >> 24))

struct messges_head {
  rt_uint16_t begin;
  rt_uint16_t length;
  rt_uint8_t type;
  rt_uint8_t trans_id;
};
typedef struct messges_head messges_head_t;

struct messges_tail {
  rt_uint16_t crc;
  rt_uint16_t end;
};
typedef struct messges_tail messges_tail_t;

static uint16_t _creat_crc16(uint8_t *buffer, uint16_t length) {
  uint16_t tmp = 0xFFFF;
  int32_t i, j, flag;

  for (i = 0; i < length; i++) {
    tmp ^= buffer[i];

    for (j = 0; j < 8; j++) {
      flag = tmp & 0x01;
      tmp >>= 1;

      if (1 == flag) tmp ^= 0xA001;
    }
  }

  return tmp;
}

//后续需要 free(buf)
rt_err_t messges_coding(rt_uint8_t *messges, rt_size_t len, rt_uint8_t type,
                        rt_uint8_t *buf, rt_uint16_t *bufsz) {
  messges_head_t *_head;
  messges_tail_t *_tail;

  if (len == 0) {
    goto _exit;
  }

  *bufsz = sizeof(messges_head_t) + len + sizeof(messges_tail_t);
  buf = rt_calloc(1, (rt_size_t)(*bufsz));
  if (buf == RT_NULL) {
    goto _exit;
  }

  _head = (messges_head_t *)buf;
  _head->begin = HTONS(MESSGES_BEGIN);
  _head->length = HTONS(*bufsz);
  _head->type = type;
  _head->trans_id = 0;

  rt_memcpy(&buf[sizeof(messges_head_t)], messges, len);
  _tail = (messges_tail_t *)&buf[*bufsz - sizeof(messges_tail_t)];

  _tail->crc = HTONS(_creat_crc16(buf, *bufsz - sizeof(messges_tail_t)));
  _tail->end = HTONS(MESSGES_END);

  return RT_EOK;

_exit:
  return -RT_EEMPTY;
}

rt_err_t messges_encoding(rt_uint8_t *messges, rt_size_t len, rt_uint8_t *type,
                          rt_uint8_t *buf, rt_uint16_t *bufsz) {
  if (*buf)
    ;
    
  messges_head_t *_head = (messges_head_t *)messges;
  *type = _head->type;
  buf = messges + sizeof(messges_head_t);
  *bufsz = len - sizeof(messges_head_t) - sizeof(messges_tail_t);
  return RT_EOK;
}
