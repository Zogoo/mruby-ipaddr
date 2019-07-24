#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "mruby.h"
#include "mruby/string.h"

union ip_addr {
  uint64_t u64[2];
  uint32_t u32[4];
  uint16_t u16[8];
  uint8_t raw[16];
};

static void
copy_htonl_ip_addr_buffer(union ip_addr *buf, mrb_int len, uint8_t *src)
{
  for (mrb_int i = 0; i < len; i += 4) {
    buf->u32[i / 4] = htonl(*(src + (i + 0)) | *(src + (i + 1)) << 8 | *(src + (i + 2)) << 16 | *(src + (i + 3)) << 24);
  }
}

static void
copy_ntohl_ip_addr_buffer(union ip_addr *buf)
{
  for (int i = 0; i < 4; i++) {
    buf->u32[i] = ntohl(buf->u32[i]);
  }
}

static mrb_value
mrb_ipaddr_op_or_ipaddr(mrb_state *mrb, mrb_value klass)
{
  mrb_int slen, olen;
  uint8_t *self, *other;
  union ip_addr sbuf = {}, obuf = {};

  mrb_get_args(mrb, "ss", (char *)&self, &slen, (char *)&other, &olen);

  if (slen != 4 && slen != 16 && olen != 4 && olen != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  copy_htonl_ip_addr_buffer(&sbuf, slen, self);
  copy_htonl_ip_addr_buffer(&obuf, olen, other);

  for (int i = 0; i < 4; i++) {
    sbuf.u32[i] = ntohl(sbuf.u32[i] | obuf.u32[i]);
  }

  return mrb_str_new(mrb, (char *)&sbuf.raw, slen);
}

static mrb_value
mrb_ipaddr_op_or_integer(mrb_state *mrb, mrb_value klass)
{
  mrb_int slen, other;
  uint8_t *self;
  union ip_addr sbuf = {};

  mrb_get_args(mrb, "si", (char *)&self, &slen, &other);

  if (slen != 4 && slen != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (slen == 4 && other > UINT32_MAX) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  copy_htonl_ip_addr_buffer(&sbuf, slen, self);

  if (slen == 4) {
    sbuf.u32[0] = sbuf.u32[0] | other;
  } else {
    sbuf.u32[0] = sbuf.u32[0] | 0ul;
    sbuf.u32[1] = sbuf.u32[1] | 0ul;
#if defined(MRB_INT64)
    sbuf.u32[2] = sbuf.u32[2] | (other >> 32);
#else
    sbuf.u32[2] = sbuf.u32[2] | 0ul;
#endif
    sbuf.u32[3] = sbuf.u32[3] | other;
  }

  copy_ntohl_ip_addr_buffer(&sbuf);

  return mrb_str_new(mrb, (char *)&sbuf.raw, slen);
}

static mrb_value
mrb_ipaddr_op_and_ipaddr(mrb_state *mrb, mrb_value klass)
{
  mrb_int slen, olen;
  uint8_t *self, *other;
  union ip_addr sbuf = {}, obuf = {};

  mrb_get_args(mrb, "ss", (char *)&self, &slen, (char *)&other, &olen);

  if (slen != 4 && slen != 16 && olen != 4 && olen != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  copy_htonl_ip_addr_buffer(&sbuf, slen, self);
  copy_htonl_ip_addr_buffer(&obuf, olen, other);

  for (int i = 0; i < 4; i++) {
    sbuf.u32[i] = ntohl(sbuf.u32[i] & obuf.u32[i]);
  }

  return mrb_str_new(mrb, (char *)&sbuf.raw, slen);
}

static mrb_value
mrb_ipaddr_op_and_integer(mrb_state *mrb, mrb_value klass)
{
  mrb_int slen, other;
  uint8_t *self;
  union ip_addr sbuf = {};

  mrb_get_args(mrb, "si", (char *)&self, &slen, &other);

  if (slen != 4 && slen != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (slen == 4 && other > UINT32_MAX) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  copy_htonl_ip_addr_buffer(&sbuf, slen, self);

  if (slen == 4) {
    sbuf.u32[0] = sbuf.u32[0] & other;
  } else {
    sbuf.u32[0] = sbuf.u32[0] & 0ul;
    sbuf.u32[1] = sbuf.u32[1] & 0ul;
#if defined(MRB_INT64)
    sbuf.u32[2] = sbuf.u32[2] & (other >> 32);
#else
    sbuf.u32[2] = sbuf.u32[2] & 0ul;
#endif
    sbuf.u32[3] = sbuf.u32[3] & other;
  }

  copy_ntohl_ip_addr_buffer(&sbuf);

  return mrb_str_new(mrb, (char *)&sbuf.raw, slen);
}

static mrb_value
mrb_ipaddr_right_shift(mrb_state *mrb, mrb_value klass)
{
  mrb_int num, n;
  uint8_t *src;
  union ip_addr buf = {};

  mrb_get_args(mrb, "is", &num, (char *)&src, &n);

  if (num < 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (n != 4 && n != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (num < 128) {
    copy_htonl_ip_addr_buffer(&buf, n, src);
  }

  if (num >= 32 && num <= 63) {
    buf.u32[3] = buf.u32[2];
    buf.u32[2] = buf.u32[1];
    buf.u32[1] = buf.u32[0];
    buf.u32[0] = 0ul;
    num -= 32;
  } else if (num >= 64 && num <= 95) {
    buf.u32[3] = buf.u32[1];
    buf.u32[2] = buf.u32[0];
    buf.u32[1] = 0ul;
    buf.u32[0] = 0ul;
    num -= 64;
  } else if (num >= 96 && num <= 127) {
    buf.u32[3] = buf.u32[0];
    buf.u32[2] = 0ul;
    buf.u32[1] = 0ul;
    buf.u32[0] = 0ul;
    num -= 96;
  }

  for (int i = 3; i >= 0; i--) {
    if (i != 0) {
      buf.u32[i] = (buf.u32[i - 1] << ((sizeof(uint32_t) * CHAR_BIT) - num)) | (buf.u32[i] >> num);
    } else {
      buf.u32[i] = (buf.u32[i] >> num);
    }

    buf.u32[i] = ntohl(buf.u32[i]);
  }

  return mrb_str_new(mrb, (char *)&buf.raw, n);
}

static mrb_value
mrb_ipaddr_left_shift(mrb_state *mrb, mrb_value klass)
{
  mrb_int num, n;
  unsigned char *src;
  union ip_addr buf = {};

  mrb_get_args(mrb, "is", &num, (char *)&src, &n);

  if (num < 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (n != 4 && n != 16) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }

  if (num < 128) {
    copy_htonl_ip_addr_buffer(&buf, n, src);
  }

  if (num >= 32 && num <= 63) {
    buf.u32[0] = buf.u32[1];
    buf.u32[1] = buf.u32[2];
    buf.u32[2] = buf.u32[3];
    buf.u32[3] = 0ul;
    num -= 32;
  } else if (num >= 64 && num <= 95) {
    buf.u32[0] = buf.u32[2];
    buf.u32[1] = buf.u32[3];
    buf.u32[2] = 0ul;
    buf.u32[3] = 0ul;
    num -= 64;
  } else if (num >= 96 && num <= 127) {
    buf.u32[0] = buf.u32[3];
    buf.u32[1] = 0ul;
    buf.u32[2] = 0ul;
    buf.u32[3] = 0ul;
    num -= 96;
  }

  for (int i = 0; i < 4; i++) {
    if (i != 3) {
      buf.u32[i] = (buf.u32[i + 1] >> ((sizeof(uint32_t) * CHAR_BIT) - num)) | (buf.u32[i] << num);
    } else {
      buf.u32[i] = (buf.u32[i] << num);
    }

    buf.u32[i] = ntohl(buf.u32[i]);
  }

  return mrb_str_new(mrb, (char *)&buf.raw, n);
}

static mrb_value
mrb_ipaddr_ntop(mrb_state *mrb, mrb_value klass)
{
  mrb_int af, n;
  char *addr, buf[50];

  mrb_get_args(mrb, "s", &addr, &n);
  if (n == 4) {
    af = AF_INET;
  } else if (n == 16) {
    af = AF_INET6;
  } else {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  }
  if (inet_ntop(af, addr, buf, sizeof(buf)) == NULL)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  return mrb_str_new_cstr(mrb, buf);
}

static mrb_value
mrb_ipaddr_pton(mrb_state *mrb, mrb_value klass)
{
  mrb_int af, n;
  mrb_value s;
  char *bp, buf[50];

  mrb_get_args(mrb, "is", &af, &bp, &n);
  if (n > sizeof(buf) - 1)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
  memcpy(buf, bp, n);
  buf[n] = '\0';

  if (af == AF_INET) {
    struct in_addr in;
    if (inet_pton(AF_INET, buf, (void *)&in.s_addr) != 1) {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
    }
    s = mrb_str_new(mrb, (char *)&in.s_addr, 4);
  } else if (af == AF_INET6) {
    struct in6_addr in6;
    if (inet_pton(AF_INET6, buf, (void *)&in6.s6_addr) != 1) {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid address");
    }
    s = mrb_str_new(mrb, (char *)&in6.s6_addr, 16);
  } else
    mrb_raise(mrb, E_ARGUMENT_ERROR, "unsupported address family");

  return s;
}

void
mrb_mruby_ipaddr_gem_init(mrb_state *mrb)
{
  struct RClass *c;

  c = mrb_define_class(mrb, "IPAddr", mrb->object_class);
  mrb_define_class_method(mrb, c, "_pton", mrb_ipaddr_pton, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "ntop", mrb_ipaddr_ntop, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_left_shift", mrb_ipaddr_left_shift, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_right_shift", mrb_ipaddr_right_shift, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_op_and_ipaddr", mrb_ipaddr_op_and_ipaddr, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_op_and_integer", mrb_ipaddr_op_and_integer, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_op_or_ipaddr", mrb_ipaddr_op_or_ipaddr, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, c, "_op_or_integer", mrb_ipaddr_op_or_integer, MRB_ARGS_REQ(1));
}

void
mrb_mruby_ipaddr_gem_final(mrb_state *mrb)
{
}
