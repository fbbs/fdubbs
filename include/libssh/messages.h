/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2009 by Aris Adamantiadis
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "config.h"

struct ssh_auth_request {
    char *username;
    int method;
    char *password;
    struct ssh_public_key_struct *public_key;
    char signature_state;
};

struct ssh_channel_request_open {
    int type;
    uint32_t sender;
    uint32_t window;
    uint32_t packet_size;
    char *originator;
    uint16_t originator_port;
    char *destination;
    uint16_t destination_port;
};

struct ssh_service_request {
    char *service;
};

struct ssh_channel_request {
    int type;
    ssh_channel channel;
    uint8_t want_reply;
    /* pty-req type specifics */
    char *TERM;
    uint32_t width;
    uint32_t height;
    uint32_t pxwidth;
    uint32_t pxheight;
    ssh_string modes;

    /* env type request */
    char *var_name;
    char *var_value;
    /* exec type request */
    char *command;
    /* subsystem */
    char *subsystem;
};

struct ssh_message_struct {
    ssh_session session;
    int type;
    struct ssh_auth_request auth_request;
    struct ssh_channel_request_open channel_request_open;
    struct ssh_channel_request channel_request;
    struct ssh_service_request service_request;
};


void message_handle(ssh_session session, uint32_t type);
int ssh_execute_message_callbacks(ssh_session session);

#endif /* MESSAGES_H_ */
