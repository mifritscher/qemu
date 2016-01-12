/*
 * QEMU I/O channels sockets driver
 *
 * Copyright (c) 2015 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QIO_CHANNEL_SOCKET_H__
#define QIO_CHANNEL_SOCKET_H__

#include "io/channel.h"
#include "io/task.h"
#include "qemu/sockets.h"

#define TYPE_QIO_CHANNEL_SOCKET "qio-channel-socket"
#define QIO_CHANNEL_SOCKET(obj)                                     \
    OBJECT_CHECK(QIOChannelSocket, (obj), TYPE_QIO_CHANNEL_SOCKET)

typedef struct QIOChannelSocket QIOChannelSocket;

/**
 * QIOChannelSocket:
 *
 * The QIOChannelSocket class provides a channel implementation
 * that can transport data over a UNIX socket or TCP socket.
 * Beyond the core channel API, it also provides functionality
 * for accepting client connections, tuning some socket
 * parameters and getting socket address strings.
 */

struct QIOChannelSocket {
    QIOChannel parent;
    int fd;
    struct sockaddr_storage localAddr;
    socklen_t localAddrLen;
    struct sockaddr_storage remoteAddr;
    socklen_t remoteAddrLen;
};


/**
 * qio_channel_socket_new:
 *
 * Create a channel for performing I/O on a socket
 * connection, that is initially closed. After
 * creating the socket, it must be setup as a client
 * connection or server.
 *
 * Returns: the socket channel object
 */
QIOChannelSocket *
qio_channel_socket_new(void);

/**
 * qio_channel_socket_new_fd:
 * @fd: the socket file descriptor
 * @errp: pointer to an uninitialized error object
 *
 * Create a channel for performing I/O on the socket
 * connection represented by the file descriptor @fd.
 *
 * Returns: the socket channel object, or NULL on error
 */
QIOChannelSocket *
qio_channel_socket_new_fd(int fd,
                          Error **errp);


/**
 * qio_channel_socket_connect_sync:
 * @ioc: the socket channel object
 * @addr: the address to connect to
 * @errp: pointer to an uninitialized error object
 *
 * Attempt to connect to the address @addr. This method
 * will run in the foreground so the caller will not regain
 * execution control until the connection is established or
 * an error occurs.
 */
int qio_channel_socket_connect_sync(QIOChannelSocket *ioc,
                                    SocketAddress *addr,
                                    Error **errp);

/**
 * qio_channel_socket_connect_async:
 * @ioc: the socket channel object
 * @addr: the address to connect to
 * @callback: the function to invoke on completion
 * @opaque: user data to pass to @callback
 * @destroy: the function to free @opaque
 *
 * Attempt to connect to the address @addr. This method
 * will run in the background so the caller will regain
 * execution control immediately. The function @callback
 * will be invoked on completion or failure.
 */
void qio_channel_socket_connect_async(QIOChannelSocket *ioc,
                                      SocketAddress *addr,
                                      QIOTaskFunc callback,
                                      gpointer opaque,
                                      GDestroyNotify destroy);


/**
 * qio_channel_socket_listen_sync:
 * @ioc: the socket channel object
 * @addr: the address to listen to
 * @errp: pointer to an uninitialized error object
 *
 * Attempt to listen to the address @addr. This method
 * will run in the foreground so the caller will not regain
 * execution control until the connection is established or
 * an error occurs.
 */
int qio_channel_socket_listen_sync(QIOChannelSocket *ioc,
                                   SocketAddress *addr,
                                   Error **errp);

/**
 * qio_channel_socket_listen_async:
 * @ioc: the socket channel object
 * @addr: the address to listen to
 * @callback: the function to invoke on completion
 * @opaque: user data to pass to @callback
 * @destroy: the function to free @opaque
 *
 * Attempt to listen to the address @addr. This method
 * will run in the background so the caller will regain
 * execution control immediately. The function @callback
 * will be invoked on completion or failure.
 */
void qio_channel_socket_listen_async(QIOChannelSocket *ioc,
                                     SocketAddress *addr,
                                     QIOTaskFunc callback,
                                     gpointer opaque,
                                     GDestroyNotify destroy);


/**
 * qio_channel_socket_dgram_sync:
 * @ioc: the socket channel object
 * @localAddr: the address to local bind address
 * @remoteAddr: the address to remote peer address
 * @errp: pointer to an uninitialized error object
 *
 * Attempt to initialize a datagram socket bound to
 * @localAddr and communicating with peer @remoteAddr.
 * This method will run in the foreground so the caller
 * will not regain execution control until the socket
 * is established or an error occurs.
 */
int qio_channel_socket_dgram_sync(QIOChannelSocket *ioc,
                                  SocketAddress *localAddr,
                                  SocketAddress *remoteAddr,
                                  Error **errp);

/**
 * qio_channel_socket_dgram_async:
 * @ioc: the socket channel object
 * @localAddr: the address to local bind address
 * @remoteAddr: the address to remote peer address
 * @callback: the function to invoke on completion
 * @opaque: user data to pass to @callback
 * @destroy: the function to free @opaque
 *
 * Attempt to initialize a datagram socket bound to
 * @localAddr and communicating with peer @remoteAddr.
 * This method will run in the background so the caller
 * will regain execution control immediately. The function
 * @callback will be invoked on completion or failure.
 */
void qio_channel_socket_dgram_async(QIOChannelSocket *ioc,
                                    SocketAddress *localAddr,
                                    SocketAddress *remoteAddr,
                                    QIOTaskFunc callback,
                                    gpointer opaque,
                                    GDestroyNotify destroy);


/**
 * qio_channel_socket_get_local_address:
 * @ioc: the socket channel object
 * @errp: pointer to an uninitialized error object
 *
 * Get the string representation of the local socket
 * address. A pointer to the allocated address information
 * struct will be returned, which the caller is required to
 * release with a call qapi_free_SocketAddress when no
 * longer required.
 *
 * Returns: 0 on success, -1 on error
 */
SocketAddress *
qio_channel_socket_get_local_address(QIOChannelSocket *ioc,
                                     Error **errp);

/**
 * qio_channel_socket_get_remote_address:
 * @ioc: the socket channel object
 * @errp: pointer to an uninitialized error object
 *
 * Get the string representation of the local socket
 * address. A pointer to the allocated address information
 * struct will be returned, which the caller is required to
 * release with a call qapi_free_SocketAddress when no
 * longer required.
 *
 * Returns: the socket address struct, or NULL on error
 */
SocketAddress *
qio_channel_socket_get_remote_address(QIOChannelSocket *ioc,
                                      Error **errp);


/**
 * qio_channel_socket_accept:
 * @ioc: the socket channel object
 * @errp: pointer to an uninitialized error object
 *
 * If the socket represents a server, then this accepts
 * a new client connection. The returned channel will
 * represent the connected client socket.
 *
 * Returns: the new client channel, or NULL on error
 */
QIOChannelSocket *
qio_channel_socket_accept(QIOChannelSocket *ioc,
                          Error **errp);


#endif /* QIO_CHANNEL_SOCKET_H__ */
