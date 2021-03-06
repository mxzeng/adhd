/* Copyright 2019 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <syslog.h>

#include "cras_iodev_list.h"
#include "cras_messages.h"
#include "cras_observer.h"
#include "cras_rclient.h"
#include "cras_rclient_util.h"
#include "cras_rstream.h"
#include "cras_types.h"
#include "cras_util.h"

/* Entry point for handling a message from the client.  Called from the main
 * server context. */
static int cpr_handle_message_from_client(struct cras_rclient *client,
					  const struct cras_server_message *msg,
					  int *fds, unsigned int num_fds)
{
	int rc = 0;
	assert(client && msg);

	rc = rclient_validate_message_fds(msg, fds, num_fds);
	if (rc < 0) {
		for (int i = 0; i < (int)num_fds; i++)
			if (fds[i] >= 0)
				close(fds[i]);
		return rc;
	}
	int fd = num_fds > 0 ? fds[0] : -1;

	switch (msg->id) {
	case CRAS_SERVER_CONNECT_STREAM: {
		int client_shm_fd = num_fds > 1 ? fds[1] : -1;
		struct cras_connect_message cmsg;
		if (MSG_LEN_VALID(msg, struct cras_connect_message)) {
			rc = rclient_handle_client_stream_connect(
				client,
				(const struct cras_connect_message *)msg, fd,
				client_shm_fd);
		} else if (!convert_connect_message_old(msg, &cmsg)) {
			rc = rclient_handle_client_stream_connect(
				client, &cmsg, fd, client_shm_fd);
		} else {
			return -EINVAL;
		}
		break;
	}
	case CRAS_SERVER_DISCONNECT_STREAM:
		if (!MSG_LEN_VALID(msg, struct cras_disconnect_stream_message))
			return -EINVAL;
		rc = rclient_handle_client_stream_disconnect(
			client,
			(const struct cras_disconnect_stream_message *)msg);
		break;
	default:
		break;
	}

	return rc;
}

/* Declarations of cras_rclient operators for cras_playback_rclient. */
static const struct cras_rclient_ops cras_playback_rclient_ops = {
	.handle_message_from_client = cpr_handle_message_from_client,
	.send_message_to_client = rclient_send_message_to_client,
	.destroy = rclient_destroy,
};

/*
 * Exported Functions.
 */

/* Creates a client structure and sends a message back informing the client that
 * the connection has succeeded. */
struct cras_rclient *cras_playback_rclient_create(int fd, size_t id)
{
	return rclient_generic_create(
		fd, id, &cras_playback_rclient_ops,
		cras_stream_direction_mask(CRAS_STREAM_OUTPUT));
}
