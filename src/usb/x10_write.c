/*
 * Copyright 2010-2011 Brian Uechi <buasst@gmail.com>
 *
 * This file is part of mochad.
 *
 * mochad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mochad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mochad.  If not, see <http://www.gnu.org/licenses/>.
 */

/* CM15A output FIFO
 * X10 I/O is very slow so store up pending output then wait for X10ACK
 * (0x55) before sending the next item from the queue.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include "global.h"
#include "transport_evidence.h"
#include "x10_write.h"

typedef struct x10out {
    size_t outlen;
    unsigned char outdata[8];
    mochad_transport_attempt attempt;
} x10out_t;

static x10out_t Outrecs[256];
#define OUTPTRSSIZE (sizeof(Outrecs) / sizeof(Outrecs[0]))
static int Outhead = 0;
static int Outtail = 0;
static int Outbusy = 0;

static int next_index(int idx) { return ((idx + 1) % OUTPTRSSIZE); }

static int add_x10out(unsigned char *buf, size_t buflen, const mochad_transport_attempt *attempt) {
    int nxt;
    x10out_t *nxtrec;

    dbprintf("len %lu\n", buflen);
    if (buflen > sizeof(Outrecs[0].outdata)) {
        dbprintf("x10 output too long %lu/%lu\n", (unsigned long)buflen,
                 (unsigned long)sizeof(Outrecs[0].outdata));
        return -1;
    }
    if ((nxt = next_index(Outtail)) == Outhead) {
        dbprintf("Outptrs full Outhead/tail %d/%d\n", Outhead, Outtail);
        mochad_transport_evidence_usb_queue_failed(attempt);
        return -1;
    }

    nxtrec = &Outrecs[nxt];
    nxtrec->outlen = buflen;
    memcpy(nxtrec->outdata, buf, buflen);
    nxtrec->attempt = *attempt;
    Outtail = nxt;
    mochad_transport_evidence_usb_queued(attempt);
    return buflen;
}

int send_next_x10out(void) {
    x10out_t *outrec;
    int next;
    int r;

    if (Outbusy) {
        dbprintf("Outhead Outtail %d/%d\n", Outhead, Outtail);
        if (Outhead == Outtail) {
            /* Empty */
            Outbusy = 0;
            PollTimeOut = -1;
        } else {
            next = next_index(Outhead);
            outrec = &Outrecs[next];
            r = write_usb(outrec->outdata, outrec->outlen);
            if (r < 0) {
                dbprintf("queued USB write failed %d\n", r);
                mochad_transport_evidence_usb_submit_failed(&outrec->attempt, r);
                outrec->attempt = mochad_transport_evidence_attempt_retry(&outrec->attempt);
                mochad_transport_evidence_usb_queued(&outrec->attempt);
                PollTimeOut = 2 * 1000;
                return r;
            }
            mochad_transport_evidence_usb_submitted(&outrec->attempt);
            Outhead = next;
        }
    }
    return 0;
}

void cancel_pending_x10out(void) {
    while (Outhead != Outtail) {
        Outhead = next_index(Outhead);
        mochad_transport_evidence_attempt_terminal_for(&Outrecs[Outhead].attempt, "cancelled",
                                                       "shutdown_before_submission");
    }
    Outbusy = 0;
    PollTimeOut = -1;
}

int x10_write(unsigned char *buf, size_t buflen) {
    int r;
    mochad_transport_attempt attempt;

    dbprintf("Outbusy=%d\n", Outbusy);
    if (buflen > sizeof(Outrecs[0].outdata)) {
        dbprintf("x10 write too long %lu/%lu\n", (unsigned long)buflen,
                 (unsigned long)sizeof(Outrecs[0].outdata));
        return -1;
    }
    attempt = mochad_transport_evidence_attempt_begin();
    if (Outbusy) {
        return add_x10out(buf, buflen, &attempt);
    } else {
        Outbusy = 1;
        mochad_transport_evidence_usb_queued(&attempt);
        r = write_usb(buf, buflen);
        if (r < 0) {
            Outbusy = 0;
            PollTimeOut = -1;
            mochad_transport_evidence_usb_submit_failed(&attempt, r);
            return r;
        }
        mochad_transport_evidence_usb_submitted(&attempt);
        PollTimeOut = 2 * 1000; /* 2 seconds */
    }
    return buflen;
}
