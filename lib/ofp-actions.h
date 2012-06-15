/*
 * Copyright (c) 2012 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OFP_ACTIONS_H
#define OFP_ACTIONS_H 1

#include <stdint.h>
#include "meta-flow.h"
#include "ofp-errors.h"
#include "ofp-util.h"
#include "openflow/openflow.h"
#include "openflow/nicira-ext.h"
#include "openvswitch/types.h"

/* List of OVS abstracted actions.
 *
 * This macro is used directly only internally by this header, but the list is
 * still of interest to developers.
 *
 * Each DEFINE_OFPACT invocation has the following parameters:
 *
 * 1. <ENUM>, used below in the enum definition of OFPACT_<ENUM>, and
 *    elsewhere.
 *
 * 2. <STRUCT> corresponding to a structure "struct <STRUCT>", that must be
 *    defined below.  This structure must be an abstract definition of the
 *    action.  Its first member must have type "struct ofpact" and name
 *    "ofpact".  It may be fixed length or end with a flexible array member
 *    (e.g. "int member[];").
 *
 * 3. <MEMBER>, which has one of two possible values:
 *
 *        - If "struct <STRUCT>" is fixed-length, it must be "ofpact".
 *
 *        - If "struct <STRUCT>" is variable-length, it must be the name of the
 *          flexible array member.
 */
#define OFPACTS                                                     \
    /* Sentinel. */                                                 \
    DEFINE_OFPACT(END,             ofpact_null,          ofpact)    \
                                                                    \
    /* Output. */                                                   \
    DEFINE_OFPACT(OUTPUT,          ofpact_output,        ofpact)    \
    DEFINE_OFPACT(CONTROLLER,      ofpact_controller,    ofpact)    \
    DEFINE_OFPACT(ENQUEUE,         ofpact_enqueue,       ofpact)    \
    DEFINE_OFPACT(OUTPUT_REG,      ofpact_output_reg,    ofpact)    \
    DEFINE_OFPACT(BUNDLE,          ofpact_bundle,        slaves)    \
                                                                    \
    /* Header changes. */                                           \
    DEFINE_OFPACT(SET_VLAN_VID,    ofpact_vlan_vid,      ofpact)    \
    DEFINE_OFPACT(SET_VLAN_PCP,    ofpact_vlan_pcp,      ofpact)    \
    DEFINE_OFPACT(STRIP_VLAN,      ofpact_null,          ofpact)    \
    DEFINE_OFPACT(SET_ETH_SRC,     ofpact_mac,           ofpact)    \
    DEFINE_OFPACT(SET_ETH_DST,     ofpact_mac,           ofpact)    \
    DEFINE_OFPACT(SET_IPV4_SRC,    ofpact_ipv4,          ofpact)    \
    DEFINE_OFPACT(SET_IPV4_DST,    ofpact_ipv4,          ofpact)    \
    DEFINE_OFPACT(SET_IPV4_DSCP,   ofpact_dscp,          ofpact)    \
    DEFINE_OFPACT(SET_L4_SRC_PORT, ofpact_l4_port,       ofpact)    \
    DEFINE_OFPACT(SET_L4_DST_PORT, ofpact_l4_port,       ofpact)    \
    DEFINE_OFPACT(REG_MOVE,        ofpact_reg_move,      ofpact)    \
    DEFINE_OFPACT(REG_LOAD,        ofpact_reg_load,      ofpact)    \
    DEFINE_OFPACT(DEC_TTL,         ofpact_null,          ofpact)    \
                                                                    \
    /* Metadata. */                                                 \
    DEFINE_OFPACT(SET_TUNNEL,      ofpact_tunnel,        ofpact)    \
    DEFINE_OFPACT(SET_QUEUE,       ofpact_queue,         ofpact)    \
    DEFINE_OFPACT(POP_QUEUE,       ofpact_null,          ofpact)    \
    DEFINE_OFPACT(FIN_TIMEOUT,     ofpact_fin_timeout,   ofpact)    \
                                                                    \
    /* Flow table interaction. */                                   \
    DEFINE_OFPACT(RESUBMIT,        ofpact_resubmit,      ofpact)    \
    DEFINE_OFPACT(LEARN,           ofpact_learn,         specs)     \
                                                                    \
    /* Arithmetic. */                                               \
    DEFINE_OFPACT(MULTIPATH,       ofpact_multipath,     ofpact)    \
    DEFINE_OFPACT(AUTOPATH,        ofpact_autopath,      ofpact)    \
                                                                    \
    /* Other. */                                                    \
    DEFINE_OFPACT(NOTE,            ofpact_note,          data)      \
    DEFINE_OFPACT(EXIT,            ofpact_null,          ofpact)

/* enum ofpact_type, with a member OFPACT_<ENUM> for each action. */
enum OVS_PACKED_ENUM ofpact_type {
#define DEFINE_OFPACT(ENUM, STRUCT, MEMBER) OFPACT_##ENUM,
    OFPACTS
#undef DEFINE_OFPACT
};

/* N_OFPACTS, the number of values of "enum ofpact_type". */
enum {
    N_OFPACTS =
#define DEFINE_OFPACT(ENUM, STRUCT, MEMBER) + 1
    OFPACTS
#undef DEFINE_OFPACT
};

/* Header for an action.
 *
 * Each action is a structure "struct ofpact_*" that begins with "struct
 * ofpact", usually followed by other data that describes the action.  Actions
 * are padded out to a multiple of OFPACT_ALIGNTO bytes in length. */
struct ofpact {
    enum ofpact_type type;      /* OFPACT_*. */
    enum ofputil_action_code compat; /* Original type when added, if any. */
    uint16_t len;               /* Length of the action, in bytes, including
                                 * struct ofpact, excluding padding. */
};

#ifdef __GNUC__
/* Make sure that OVS_PACKED_ENUM really worked. */
BUILD_ASSERT_DECL(sizeof(struct ofpact) == 4);
#endif

/* Alignment. */
#define OFPACT_ALIGNTO 8
#define OFPACT_ALIGN(SIZE) ROUND_UP(SIZE, OFPACT_ALIGNTO)

static inline struct ofpact *
ofpact_next(const struct ofpact *ofpact)
{
    return (void *) ((uint8_t *) ofpact + OFPACT_ALIGN(ofpact->len));
}

/* Assigns POS to each action starting at OFPACTS in turn, assuming that the
 * set of actions is terminated by OFPACT_END. */
#define OFPACT_FOR_EACH(POS, OFPACTS)                           \
    for ((POS) = (OFPACTS); (POS)->type != OFPACT_END;          \
         (POS) = ofpact_next(POS))

/* Action structure for each OFPACT_*. */

/* OFPACT_END, OFPACT_STRIP_VLAN, OFPACT_DEC_TTL, OFPACT_POP_QUEUE,
 * OFPACT_EXIT.
 *
 * Used for OFPAT10_STRIP_VLAN, NXAST_DEC_TTL, NXAST_POP_QUEUE, NXAST_EXIT.
 *
 * Action structure for actions that do not have any extra data beyond the
 * action type. */
struct ofpact_null {
    struct ofpact ofpact;
};

/* OFPACT_OUTPUT.
 *
 * Used for OFPAT10_OUTPUT. */
struct ofpact_output {
    struct ofpact ofpact;
    uint16_t port;              /* Output port. */
    uint16_t max_len;           /* Max send len, for port OFPP_CONTROLLER. */
};

/* OFPACT_CONTROLLER.
 *
 * Used for NXAST_CONTROLLER. */
struct ofpact_controller {
    struct ofpact ofpact;
    uint16_t max_len;           /* Maximum length to send to controller. */
    uint16_t controller_id;     /* Controller ID to send packet-in. */
    enum ofp_packet_in_reason reason; /* Reason to put in packet-in. */
};

/* OFPACT_ENQUEUE.
 *
 * Used for OFPAT10_ENQUEUE. */
struct ofpact_enqueue {
    struct ofpact ofpact;
    uint16_t port;
    uint32_t queue;
};

/* OFPACT_OUTPUT_REG.
 *
 * Used for NXAST_OUTPUT_REG. */
struct ofpact_output_reg {
    struct ofpact ofpact;
    struct mf_subfield src;
    uint16_t max_len;
};

/* OFPACT_BUNDLE.
 *
 * Used for NXAST_BUNDLE. */
struct ofpact_bundle {
    struct ofpact ofpact;

    /* Slave choice algorithm to apply to hash value. */
    enum nx_bd_algorithm algorithm;

    /* What fields to hash and how. */
    enum nx_hash_fields fields;
    uint16_t basis;             /* Universal hash parameter. */

    struct mf_subfield dst;

    /* Slaves for output. */
    unsigned int n_slaves;
    uint16_t slaves[];
};

/* OFPACT_SET_VLAN_VID.
 *
 * Used for OFPAT10_SET_VLAN_VID. */
struct ofpact_vlan_vid {
    struct ofpact ofpact;
    uint16_t vlan_vid;          /* VLAN VID in low 12 bits, 0 in other bits. */
};

/* OFPACT_SET_VLAN_PCP.
 *
 * Used for OFPAT10_SET_VLAN_PCP. */
struct ofpact_vlan_pcp {
    struct ofpact ofpact;
    uint8_t vlan_pcp;           /* VLAN PCP in low 3 bits, 0 in other bits. */
};

/* OFPACT_SET_ETH_SRC, OFPACT_SET_ETH_DST.
 *
 * Used for OFPAT10_SET_DL_SRC, OFPAT10_SET_DL_DST. */
struct ofpact_mac {
    struct ofpact ofpact;
    uint8_t mac[ETH_ADDR_LEN];
};

/* OFPACT_SET_IPV4_SRC, OFPACT_SET_IPV4_DST.
 *
 * Used for OFPAT10_SET_NW_SRC, OFPAT10_SET_NW_DST. */
struct ofpact_ipv4 {
    struct ofpact ofpact;
    ovs_be32 ipv4;
};

/* OFPACT_SET_IPV4_DSCP.
 *
 * Used for OFPAT10_SET_NW_TOS. */
struct ofpact_dscp {
    struct ofpact ofpact;
    uint8_t dscp;               /* DSCP in high 6 bits, rest ignored. */
};

/* OFPACT_SET_L4_SRC_PORT, OFPACT_SET_L4_DST_PORT.
 *
 * Used for OFPAT10_SET_TP_SRC, OFPAT10_SET_TP_DST. */
struct ofpact_l4_port {
    struct ofpact ofpact;
    uint16_t port;              /* TCP or UDP port number. */
};

/* OFPACT_REG_MOVE.
 *
 * Used for NXAST_REG_MOVE. */
struct ofpact_reg_move {
    struct ofpact ofpact;
    struct mf_subfield src;
    struct mf_subfield dst;
};

/* OFPACT_REG_LOAD.
 *
 * Used for NXAST_REG_LOAD. */
struct ofpact_reg_load {
    struct ofpact ofpact;
    struct mf_subfield dst;
    uint64_t value;
};

/* OFPACT_SET_TUNNEL.
 *
 * Used for NXAST_SET_TUNNEL, NXAST_SET_TUNNEL64. */
struct ofpact_tunnel {
    struct ofpact ofpact;
    uint64_t tun_id;
};

/* OFPACT_SET_QUEUE.
 *
 * Used for NXAST_SET_QUEUE. */
struct ofpact_queue {
    struct ofpact ofpact;
    uint32_t queue_id;
};

/* OFPACT_FIN_TIMEOUT.
 *
 * Used for NXAST_FIN_TIMEOUT. */
struct ofpact_fin_timeout {
    struct ofpact ofpact;
    uint16_t fin_idle_timeout;
    uint16_t fin_hard_timeout;
};

/* OFPACT_RESUBMIT.
 *
 * Used for NXAST_RESUBMIT, NXAST_RESUBMIT_TABLE. */
struct ofpact_resubmit {
    struct ofpact ofpact;
    uint16_t in_port;
    uint8_t table_id;
};

/* Part of struct ofpact_learn, below. */
struct ofpact_learn_spec {
    int n_bits;

    int src_type;
    struct mf_subfield src;
    union mf_subvalue src_imm;

    int dst_type;
    struct mf_subfield dst;
};

/* OFPACT_LEARN.
 *
 * Used for NXAST_LEARN. */
struct ofpact_learn {
    struct ofpact ofpact;

    uint16_t idle_timeout;      /* Idle time before discarding (seconds). */
    uint16_t hard_timeout;      /* Max time before discarding (seconds). */
    uint16_t priority;          /* Priority level of flow entry. */
    uint64_t cookie;            /* Cookie for new flow. */
    uint16_t flags;             /* Either 0 or OFPFF_SEND_FLOW_REM. */
    uint8_t table_id;           /* Table to insert flow entry. */
    uint16_t fin_idle_timeout;  /* Idle timeout after FIN, if nonzero. */
    uint16_t fin_hard_timeout;  /* Hard timeout after FIN, if nonzero. */

    unsigned int n_specs;
    struct ofpact_learn_spec specs[];
};

/* OFPACT_MULTIPATH.
 *
 * Used for NXAST_MULTIPATH. */
struct ofpact_multipath {
    struct ofpact ofpact;

    /* What fields to hash and how. */
    enum nx_hash_fields fields;
    uint16_t basis;             /* Universal hash parameter. */

    /* Multipath link choice algorithm to apply to hash value. */
    enum nx_mp_algorithm algorithm;
    uint16_t max_link;          /* Number of output links, minus 1. */
    uint32_t arg;               /* Algorithm-specific argument. */

    /* Where to store the result. */
    struct mf_subfield dst;
};

/* OFPACT_AUTOPATH.
 *
 * Used for NXAST_AUTOPATH. */
struct ofpact_autopath {
    struct ofpact ofpact;
    struct mf_subfield dst;
    uint32_t port;
};

/* OFPACT_NOTE.
 *
 * Used for NXAST_NOTE. */
struct ofpact_note {
    struct ofpact ofpact;
    size_t length;
    uint8_t data[];
};

/* Converting OpenFlow to ofpacts. */
enum ofperr ofpacts_pull_openflow(struct ofpbuf *openflow,
                                  unsigned int actions_len,
                                  struct ofpbuf *ofpacts);
enum ofperr ofpacts_check(const struct ofpact[],
                          const struct flow *, int max_ports);

/* Converting ofpacts to OpenFlow. */
void ofpacts_to_openflow(const struct ofpact[], struct ofpbuf *openflow);

/* Working with ofpacts. */
bool ofpacts_output_to_port(const struct ofpact[], uint16_t port);
bool ofpacts_equal(const struct ofpact a[], size_t a_len,
                   const struct ofpact b[], size_t b_len);

/* Formatting ofpacts.
 *
 * (For parsing ofpacts, see ofp-parse.h.) */
void ofpacts_format(const struct ofpact[], struct ds *);

/* Internal use by the helpers below. */
void ofpact_init(struct ofpact *, enum ofpact_type, size_t len);
void *ofpact_put(struct ofpbuf *, enum ofpact_type, size_t len);

/* For each OFPACT_<ENUM> with a corresponding struct <STRUCT>, this defines
 * the following commonly useful functions:
 *
 *   struct <STRUCT> *ofpact_put_<ENUM>(struct ofpbuf *ofpacts);
 *
 *     Appends a new 'ofpact', of length OFPACT_<ENUM>_RAW_SIZE, to 'ofpacts',
 *     initializes it with ofpact_init_<ENUM>(), and returns it.  Also sets
 *     'ofpacts->l2' to the returned action.
 *
 *     After using this function to add a variable-length action, add the
 *     elements of the flexible array (e.g. with ofpbuf_put()), then use
 *     ofpact_update_len() to update the length embedded into the action.
 *     (Keep in mind the need to refresh the structure from 'ofpacts->l2' after
 *     adding data to 'ofpacts'.)
 *
 *   struct <STRUCT> *ofpact_get_<ENUM>(const struct ofpact *ofpact);
 *
 *     Returns 'ofpact' cast to "struct <STRUCT> *".  'ofpact->type' must be
 *     OFPACT_<ENUM>.
 *
 * as well as the following more rarely useful definitions:
 *
 *   void ofpact_init_<ENUM>(struct <STRUCT> *ofpact);
 *
 *     Initializes the parts of 'ofpact' that identify it as having type
 *     OFPACT_<ENUM> and length OFPACT_<ENUM>_RAW_SIZE and zeros the rest.
 *
 *   <ENUM>_RAW_SIZE
 *
 *     The size of the action structure.  For a fixed-length action, this is
 *     sizeof(struct <STRUCT>).  For a variable-length action, this is the
 *     offset to the variable-length part.
 *
 *   <ENUM>_SIZE
 *
 *     An integer constant, the value of OFPACT_<ENUM>_RAW_SIZE rounded up to a
 *     multiple of OFPACT_ALIGNTO.
 */
#define DEFINE_OFPACT(ENUM, STRUCT, MEMBER)                             \
    BUILD_ASSERT_DECL(offsetof(struct STRUCT, ofpact) == 0);            \
                                                                        \
    enum { OFPACT_##ENUM##_RAW_SIZE                                     \
           = (offsetof(struct STRUCT, MEMBER)                           \
              ? offsetof(struct STRUCT, MEMBER)                         \
              : sizeof(struct STRUCT)) };                               \
                                                                        \
    enum { OFPACT_##ENUM##_SIZE                                         \
           = ROUND_UP(OFPACT_##ENUM##_RAW_SIZE, OFPACT_ALIGNTO) };      \
                                                                        \
    static inline struct STRUCT *                                       \
    ofpact_get_##ENUM(const struct ofpact *ofpact)                      \
    {                                                                   \
        assert(ofpact->type == OFPACT_##ENUM);                          \
        return (struct STRUCT *) ofpact;                                \
    }                                                                   \
                                                                        \
    static inline struct STRUCT *                                       \
    ofpact_put_##ENUM(struct ofpbuf *ofpacts)                           \
    {                                                                   \
        return ofpact_put(ofpacts, OFPACT_##ENUM,                       \
                          OFPACT_##ENUM##_RAW_SIZE);                    \
    }                                                                   \
                                                                        \
    static inline void                                                  \
    ofpact_init_##ENUM(struct STRUCT *ofpact)                           \
    {                                                                   \
        ofpact_init(&ofpact->ofpact, OFPACT_##ENUM,                     \
                    OFPACT_##ENUM##_RAW_SIZE);                          \
    }
OFPACTS
#undef DEFINE_OFPACT

void ofpact_update_len(struct ofpbuf *, struct ofpact *);

#endif /* ofp-actions.h */