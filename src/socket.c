/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_DISABLE_SOCKETS

#include "internal.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define close(x) closesocket(x)
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#endif

#define RECVTYPE_STRING 1
#define RECVTYPE_RAW 2

/*
 * Notes to review: structure below
 * is subject to change, don't pay attention on it
 * I'll remove this comment when it'll be ready
 */
struct socket_internal {
  int socket;
  int type;
  int recvtype;
  int local_port;
  int family;
};

static int get_sockerror() {
#ifdef _WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

/*
 * Socket(family, type, recvtype)
 * Defaults: family = AF_INET, type = SOCK_STREAM,
 * recvtype = STRING
 */
static v7_val_t Socket_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long arg_count;
  struct socket_internal si;

  if (!v7_is_object(this_obj) || this_obj == v7->global_object) {
    throw_exception(v7, TYPE_ERROR, "Socket ctor called as function");
  }

  memset(&si, 0, sizeof(si));

  arg_count = v7_array_length(v7, args);

  si.family = AF_INET;
  si.type = SOCK_STREAM;
  si.recvtype = RECVTYPE_STRING;

  switch (arg_count) {
    case 3:
      si.recvtype = i_as_num(v7, v7_array_get(v7, args, 2));
      if (si.recvtype != RECVTYPE_STRING && si.recvtype != RECVTYPE_RAW) {
        throw_exception(v7, TYPE_ERROR, "Invalid RecvType paramater");
      }
    case 2:
      si.type = i_as_num(v7, v7_array_get(v7, args, 1));
      if (si.type != SOCK_STREAM && si.type != SOCK_DGRAM) {
        throw_exception(v7, TYPE_ERROR, "Invalid Type parameter");
      }
    case 1:
      si.family = i_as_num(v7, v7_array_get(v7, args, 0));
      if (si.family != AF_INET && si.family != AF_INET6) {
        throw_exception(v7, TYPE_ERROR, "Invalid Family parameter");
      }
  }

  si.socket = socket(si.family, si.type, 0);

  if (si.socket < 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot create socket (%d)",
                    get_sockerror());
  }

  {
    val_t si_val;
    struct socket_internal *psi =
        (struct socket_internal *) malloc(sizeof(*psi));
    memcpy(psi, &si, sizeof(*psi));

    si_val = v7_create_foreign(psi);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, si_val);
  }

  return this_obj;
}

static struct socket_internal *Socket_check_and_get_si(struct v7 *v7,
                                                       val_t this_obj) {
  struct socket_internal *si = NULL;
  struct v7_property *si_prop =
      v7_get_own_property2(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN);

  si = (struct socket_internal *) v7_to_foreign(
      v7_property_value(v7, this_obj, si_prop));

  if (si == NULL) {
    throw_exception(v7, TYPE_ERROR, "Socket is closed");
  }

  return si;
}

static void Socket_getlocal_sockaddr(struct v7 *v7, struct socket_internal *si,
                                     struct sockaddr *sa) {
  memset(sa, 0, sizeof(*sa));

  switch (si->family) {
    case AF_INET: {
      struct sockaddr_in *sa4 = (struct sockaddr_in *) sa;
      sa4->sin_family = si->family;
      sa4->sin_port = htons(si->local_port);
      sa4->sin_addr.s_addr = INADDR_ANY;
      break;
    }
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;
      sa6->sin6_family = si->family;
      sa6->sin6_port = htons(si->local_port);
      sa6->sin6_addr = in6addr_any;
    }
#endif
    default:
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }
}

#ifdef V7_ENABLE_GETADDRINFO
static void Socket_getremote_sockaddr(struct v7 *v7, char *addr, uint16_t port,
                                      struct sockaddr *sa) {
  struct addrinfo *ai;

  if (getaddrinfo(addr, 0, 0, &ai) != 0) {
    throw_exception(v7, TYPE_ERROR, "Invalid host name");
  }

  switch (ai->ai_family) {
    case AF_INET: {
      struct sockaddr_in *psa = (struct sockaddr_in *) ai[0].ai_addr;
      psa->sin_port = htons(port);
      memcpy(sa, psa, sizeof(*psa));
      break;
    };
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      /* TODO(alashkin): verify IPv6 [my provider doesn't support it] */
      struct sockaddr_in6 *psa = (struct sockaddr_in6 *) ai[0].ai_addr;
      psa->sin6_port = htons(port);
      memcpy(sa, psa, sizeof(*psa));
      break;
    }
#endif
    default:
      freeaddrinfo(ai);
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }

  freeaddrinfo(ai);
}
#else
static void Socket_getremote_sockaddr(struct v7 *v7, char *addr, uint16_t port,
                                      struct sockaddr *sa) {
  struct hostent *host = gethostbyname(addr);
  memset(sa, 0, sizeof(*sa));

  if (host == NULL) {
    throw_exception(v7, TYPE_ERROR, "Invalid host name");
  }

  switch (host->h_addrtype) {
    case AF_INET: {
      struct sockaddr_in *psa = (struct sockaddr_in *) sa;
      psa->sin_port = htons(port);
      psa->sin_family = AF_INET;
      memcpy(&psa->sin_addr.s_addr, host->h_addr_list[0],
             sizeof(psa->sin_addr.s_addr));
      break;
    };
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      /* TODO(alashkin): verify IPv6 [my provider doesn't support it] */
      struct sockaddr_in6 *psa = (struct sockaddr_in6 *) sa;
      psa->sin6_port = htons(port);
      psa->sin_family = AF_INET6;
      memcpy(&psa->sin6_addr, host->h_addr_list[0], sizeof(psa->sin6_addr));

      break;
    }
#endif
    default:
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }
}
#endif

uint16_t Socket_check_and_get_port(struct v7 *v7, val_t port_val) {
  double port_number = i_as_num(v7, port_val);

  if (isnan(port_number) || port_number < 0 || port_number > 0xFFFF) {
    throw_exception(v7, TYPE_ERROR, "Invalid port number");
  }
  return (uint16_t) port_number;
}

/*
 * Associates a local address with a socket.
 * JS: var s = new Socket(); s.bind(80)
 * TODO(alashkin): add address as second parameter
 */
static v7_val_t Socket_bind(struct v7 *v7, val_t this_obj, val_t args) {
  struct sockaddr sa;
  long arg_count;

  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);

  arg_count = v7_array_length(v7, args);

  if (arg_count < 1 && si->local_port == 0) {
    throw_exception(v7, TYPE_ERROR,
                    "Cannot bind socket: no local port specified");
  }

  si->local_port = Socket_check_and_get_port(v7, v7_array_get(v7, args, 0));

  Socket_getlocal_sockaddr(v7, si, &sa);
  if (bind(si->socket, &sa, sizeof(sa)) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot bind socket (%d)", get_sockerror());
  }

  return this_obj;
}

/*
 * Places a socket in a state in which it is listening
 * for an incoming connection.
 * JS: var x = new Socket().... x.listen()
 */
static v7_val_t Socket_listen(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  (void) args;

  if (listen(si->socket, SOMAXCONN) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot start listening (%d)",
                    get_sockerror());
  }

  return this_obj;
}

static uint8_t *Sockey_JSarray_to_Carray(struct v7 *v7, val_t arr,
                                         size_t *buf_size) {
  uint8_t *retval, *ptr;
  unsigned long i, elem_count = v7_array_length(v7, arr);
  /* Support byte array only */
  *buf_size = elem_count * sizeof(uint8_t);
  retval = ptr = (uint8_t *) malloc(*buf_size);

  for (i = 0; i < elem_count; i++) {
    double elem = i_as_num(v7, v7_array_get(v7, arr, i));
    if (isnan(elem) || elem < 0 || elem > 0xFF) {
      break;
    }
    *ptr = (uint8_t) elem;
    ptr++;
  }

  if (i != elem_count) {
    free(retval);
    throw_exception(v7, TYPE_ERROR, "Parameter should be a byte array");
  }

  return retval;
}

static uint8_t *Socket_get_send_buf(struct v7 *v7, val_t buf_val,
                                    size_t *buf_size, int *free_buf) {
  uint8_t *retval = NULL;

  if (v7_is_string(buf_val)) {
    retval = (uint8_t *) v7_to_string(v7, &buf_val, buf_size);
    *free_buf = 0;
  } else if (is_prototype_of(v7, buf_val, v7->array_prototype)) {
    retval = Sockey_JSarray_to_Carray(v7, buf_val, buf_size);
    *free_buf = 1;
  }

  return retval;
}

/*
 * Sends data on a connected socket.
 * JS: Socket.send(buf)
 * Ex: var x = new Socket().... x.send("Hello, world!")
 */
static v7_val_t Socket_send(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  uint8_t *buf = NULL;
  size_t buf_size = 0;
  long bytes_sent;
  int free_buf = 0;

  if (v7_array_length(v7, args) != 0) {
    buf = Socket_get_send_buf(v7, v7_array_get(v7, args, 0), &buf_size,
                              &free_buf);
  }

  if (buf == NULL || buf_size == 0) {
    throw_exception(v7, TYPE_ERROR, "Invalid data to send");
  }

  bytes_sent = send(si->socket, buf, buf_size, 0);

  if (free_buf) {
    free(buf);
  }

  if (bytes_sent < 0) {
    throw_exception(v7, TYPE_ERROR, "Connot send data (%d)", get_sockerror());
  }

  return v7_create_number(bytes_sent);
}

/*
 * Establishes a connection.
 * JS: Socket.connect(addr, port)
 * Ex: var x = new Socket(); x.connect("www.hello.com",80);
 */
static v7_val_t Socket_connect(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  char addr[100] = {0};
  struct sockaddr sa;
  uint16_t port;

  if (v7_array_length(v7, args) != 2) {
    throw_exception(v7, TYPE_ERROR, "Invalid arguments count");
  }

  {
    val_t addr_val;
    size_t addr_size = 0;
    const char *addr_pointer = 0;
    addr_val = v7_array_get(v7, args, 0);
    if (v7_is_string(addr_val)) {
      addr_pointer = v7_to_string(v7, &addr_val, &addr_size);
    }
    if (addr_pointer == NULL || addr_size > sizeof(addr)) {
      throw_exception(v7, TYPE_ERROR, "Invalid address");
    }
    strncpy(addr, addr_pointer, addr_size);
  }

  port = Socket_check_and_get_port(v7, v7_array_get(v7, args, 1));
  Socket_getremote_sockaddr(v7, addr, port, &sa);

  if (connect(si->socket, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot connect (%d)", get_sockerror());
  }

  return this_obj;
}

/*
 * Closes a socket.
 * JS: Socket.close();
 * Ex: var x = new Socket(); .... x.close()
 */
static v7_val_t Socket_close(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  (void) args;

  close(si->socket);
  free(si);

  v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN,
                  v7_create_undefined());

  return this_obj;
}

V7_PRIVATE void init_socket(struct v7 *v7) {
  val_t socket =
      v7_create_cfunction_ctor(v7, v7->socket_prototype, Socket_ctor, 3);
  v7_set_property(v7, v7->global_object, "Socket", 6, V7_PROPERTY_DONT_ENUM,
                  socket);

  set_cfunc_prop(v7, v7->socket_prototype, "close", Socket_close);
  set_cfunc_prop(v7, v7->socket_prototype, "bind", Socket_bind);
  set_cfunc_prop(v7, v7->socket_prototype, "listen", Socket_listen);
  set_cfunc_prop(v7, v7->socket_prototype, "send", Socket_send);
  set_cfunc_prop(v7, v7->socket_prototype, "connect", Socket_connect);

  {
    val_t family = v7_create_object(v7);
    v7_set_property(v7, socket, "Family", 6, 0, family);
    v7_set_property(v7, family, "AF_INET", 7, 0, v7_create_number(AF_INET));
#ifdef V7_ENABLE_IPV6
    v7_set_property(v7, family, "AF_INET6", 8, 0, v7_create_number(AF_INET6));
#endif
  }

  {
    val_t type = v7_create_object(v7);
    v7_set_property(v7, socket, "Type", 4, 0, type);
    v7_set_property(v7, type, "SOCK_STREAM", 11, 0,
                    v7_create_number(SOCK_STREAM));
    v7_set_property(v7, type, "SOCK_DGRAM", 10, 0,
                    v7_create_number(SOCK_DGRAM));
  }

  {
    val_t recvtype = v7_create_object(v7);
    v7_set_property(v7, socket, "RecvType", 8, 0, recvtype);
    v7_set_property(v7, recvtype, "STRING", 6, 0,
                    v7_create_number(RECVTYPE_STRING));
    v7_set_property(v7, recvtype, "RAW", 3, 0, v7_create_number(RECVTYPE_RAW));
  }
#ifdef _WIN32
  {
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
    /* TODO(alashkin): add WSACleanup call */
  }
#else
  signal(SIGPIPE, SIG_IGN);
#endif
}

#endif
