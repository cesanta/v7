/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <Ws2ipdef.h>
#include <Windows.h>
#define close(x) closesocket(x)
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
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
  struct sockaddr remote_addr;
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

  si = (struct socket_internal *) v7_to_foreign(si_prop->value);

  if (si == NULL) {
    throw_exception(v7, TYPE_ERROR, "Socket is closed");
  }

  return si;
}

static void Socket_compose_sockaddr(struct v7 *v7, struct socket_internal *si,
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
    case AF_INET6: {
      struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;
      sa6->sin6_family = si->family;
      sa6->sin6_port = htons(si->local_port);
      sa6->sin6_addr = in6addr_any;
    }
    default:
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }
}

/*
 * Associates a local address with a socket.
 * JS: var s = new Socket(); s.bind(80)
 * TODO(alashkin): add address as second parameter
 */
static v7_val_t Socket_bind(struct v7 *v7, val_t this_obj, val_t args) {
  struct sockaddr sa;
  long arg_count;
  double port_number;

  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);

  arg_count = v7_array_length(v7, args);

  if (arg_count < 1 && si->local_port == 0) {
    throw_exception(v7, TYPE_ERROR,
                    "Cannot bind socket: no local port specified");
  }

  port_number = i_as_num(v7, v7_array_get(v7, args, 0));
  if (isnan(port_number) || port_number < 0 ||
      trunc(port_number) != port_number) {
    throw_exception(v7, TYPE_ERROR, "Invalid port number");
  }

  si->local_port = port_number;

  Socket_compose_sockaddr(v7, si, &sa);
  if (bind(si->socket, &sa, sizeof(sa)) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot bind socket (%d)", get_sockerror());
  }

  return this_obj;
}

/* Closes a socket. JS: var s = new Socket() ... s.close() */
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

  {
    val_t family = v7_create_object(v7);
    v7_set_property(v7, socket, "Family", 6, 0, family);
    v7_set_property(v7, family, "AF_INET", 7, 0, v7_create_number(AF_INET));
    v7_set_property(v7, family, "AF_INET6", 8, 0, v7_create_number(AF_INET6));
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
