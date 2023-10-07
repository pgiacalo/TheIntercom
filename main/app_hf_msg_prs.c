/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/*
The `app_hf_msf_prs.c` file appears to focus on the parsing and processing of messages in a particular format. 
The expected message format begins with a header (`hf `) and ends with a tail (`;`). 
The messages can contain commands which have associated handlers.

Let's break down the main features of the file:

1. Constants and Variables:
    - The message header `hf_msg_hdr` has a length of 3 characters (`hf `).
    - The message tail `hf_msg_tail` is a single character (`;`).

2. hf_msg_parser_reset_state:
    - This function resets the parsing state of the provided `hf_msg_prs_cb_t` structure. 
    The parser's state is set to idle, and counters/indexes are reset to their initial values.

3. hf_msg_parser_register_callback:
    - Registers a callback function that will be invoked once a valid message has been parsed.

4. hf_msg_parse:
    - This function parses a single character `c` and advances the state of the parser according to the character.
    - The parser operates in different states: Idle, Parsing the header, Parsing the payload.
    - Upon successful parsing of a message, the registered callback is invoked with the parsed message.
    - Errors can occur during parsing, like buffer overflow, header sync failure, and more. 
    These errors are returned as specific error codes.

5. hf_msg_split_args:
    - Takes a string (between start and end pointers) and splits it based on spaces, storing the start 
    of each argument in the `argv` array. The number of detected arguments is stored in `argn`.
    - This function is useful for command parsing.

6. hf_msg_args_parser:
    - Parses the arguments from the message buffer.
    - After splitting the message into individual arguments using `hf_msg_split_args`, 
    it looks up the command (first argument) in a command table. If the command exists 
    in the table and has an associated handler, the handler is called with the parsed arguments.
    - If a command is not supported, an "unsupported command" message is printed, and usage information is displayed.

Key Concepts:
- State Machine: The message parser operates as a simple state machine, transitioning between states based on 
input characters and the expected message format.
- Callback Mechanism: Once a valid message is parsed, a previously registered callback function is executed. 
This design allows for modularity and separation of concerns.
- Command Processing: Commands and their handlers are separated, likely defined in a table in another file (`app_hf_msg_set.h`). 
This design allows for easy addition and removal of supported commands.

Overall, the code is designed for parsing and processing messages in a `hf ;` format. The messages are processed one character at a time, and once a full message is received, it's processed as a command with arguments.
*/
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "app_hf_msg_prs.h"
#include "app_hf_msg_set.h"

// according to the design, message header length shall be no less than 2.
#define HF_MSG_HDR_LEN        (3)
const static char hf_msg_hdr[HF_MSG_HDR_LEN] = {'h', 'f', ' '};

// according to the design, message header length shall be no less than 2.
#define HF_MSG_TAIL_LEN       (1)
const static char hf_msg_tail[HF_MSG_TAIL_LEN] = {';'};

void hf_msg_parser_reset_state(hf_msg_prs_cb_t *prs)
{
    prs->state = HF_MSG_PRS_IDLE;
    prs->cnt = 0;
    prs->h_idx = 0;
    prs->t_idx = 0;
}

void hf_msg_parser_register_callback(hf_msg_prs_cb_t *prs, hf_msg_callback cb)
{
    prs->callback = cb;
}

hf_msg_prs_err_t hf_msg_parse(char c, hf_msg_prs_cb_t *prs)
{
    hf_msg_prs_err_t err = HF_MSG_PRS_ERR_IN_PROGRESS;
    switch (prs->state)
    {
        case HF_MSG_PRS_IDLE:
        {
            if (c == hf_msg_hdr[0]) {
                prs->state = HF_MSG_PRS_HDR;
                prs->buf[0] = c;
                prs->cnt = 1;
                prs->h_idx = 1;
            } else {
                err = HF_MSG_PRS_ERR_HDR_UNDETECTED;
            }
        }
        break;

        case HF_MSG_PRS_HDR:
        {
            if (c == hf_msg_hdr[prs->h_idx]) {
                prs->buf[prs->cnt++] = c;
                if (++(prs->h_idx) == HF_MSG_HDR_LEN) {
                    prs->state = HF_MSG_PRS_PAYL;
                    prs->t_idx = 0;
                }
            } else {
                hf_msg_parser_reset_state(prs);
                err = HF_MSG_PRS_ERR_HDR_SYNC_FAILED;
            }
        }
        break;

        case HF_MSG_PRS_PAYL:
        {
            prs->buf[prs->cnt++] = c;
            if (c == hf_msg_tail[prs->t_idx]) {
                if (++(prs->t_idx) == HF_MSG_TAIL_LEN) {
                    prs->buf[prs->cnt] = '\0';
                    prs->callback(prs->buf, prs->cnt);
                    hf_msg_parser_reset_state(prs);
                    err = HF_MSG_PRS_ERR_OK;
                    break;
                }
            } else {
                prs->t_idx = 0;
            }

            if (prs->cnt >= HF_MSG_LEN_MAX) {
                hf_msg_parser_reset_state(prs);
                err = HF_MSG_PRS_ERR_BUF_OVERFLOW;
            }
        }
        break;
    }
    return err;
}


void hf_msg_split_args(char *start, char *end, char **argv, int *argn)
{
    if (argn == NULL || *argn == 0) {
        return;
    }

    memset(argv, 0, (*argn) * sizeof(void *));

    int max_argn = *argn;
    *argn = 0;

    char *p = start;
    for (int i = 0; i < max_argn; ++i) {
        while (isspace((int)*p) && p != end) {
            ++p;
        }
        if (p == end) {
            return;
        }

        argv[i] = p++;
        ++(*argn);

        while (!isspace((int)*p) && p != end) {
            ++p;
        }

        if (p == end) {
            return;
        } else {
            *p = '\0';
            ++p;
        }
    }
}

void hf_msg_args_parser(char *buf, int len)
{
    char *argv[HF_MSG_ARGS_MAX];
    int argn = HF_MSG_ARGS_MAX;
    char *start = buf + HF_MSG_HDR_LEN;
    // set the command terminitor to '\0'
    char *end = buf + len - HF_MSG_TAIL_LEN;
    *end = '\0';

    hf_msg_split_args(start, end, argv, &argn);

    if (argn == 0) {
        return;
    }

    bool cmd_supported = false;

    hf_msg_hdl_t *cmd_tbl = hf_get_cmd_tbl();
    size_t cmd_tbl_size = hf_get_cmd_tbl_size();
    for (int i = 0; i < cmd_tbl_size; ++i) {
        hf_msg_hdl_t *hdl = &cmd_tbl[i];
        if (strcmp(argv[0], hdl->str) == 0) {
            if (hdl->handler) {
                hdl->handler(argn, argv);
                cmd_supported = true;
                break;
            }
        }
    }
    if (!cmd_supported) {
        printf("unsupported command\n");
        hf_msg_show_usage();
    }
    return;
}
