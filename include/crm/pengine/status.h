/*
 * Copyright 2004-2018 Andrew Beekhof <andrew@beekhof.net>
 *
 * This source code is licensed under the GNU Lesser General Public License
 * version 2.1 or later (LGPLv2.1+) WITHOUT ANY WARRANTY.
 */

#ifndef PENGINE_STATUS__H
#  define PENGINE_STATUS__H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Cluster status and scheduling
 * \ingroup pengine
 */

#  include <glib.h>
#  include <stdbool.h>
#  include <crm/common/iso8601.h>
#  include <crm/pengine/common.h>

typedef struct pe_node_s pe_node_t;
typedef struct pe_action_s pe_action_t;
typedef struct pe_resource_s pe_resource_t;
typedef struct pe_working_set_s pe_working_set_t;

#  include <crm/pengine/complex.h>

enum pe_quorum_policy {
    no_quorum_freeze,
    no_quorum_stop,
    no_quorum_ignore,
    no_quorum_suicide
};

enum node_type {
    node_ping,
    node_member,
    node_remote
};

//! \deprecated will be removed in a future release
enum pe_restart {
    pe_restart_restart,
    pe_restart_ignore
};

//! Determine behavior of pe_find_resource_with_flags()
enum pe_find {
    pe_find_renamed  = 0x001, //!< match resource ID or LRM history ID
    pe_find_anon     = 0x002, //!< match base name of anonymous clone instances
    pe_find_clone    = 0x004, //!< match only clone instances
    pe_find_current  = 0x008, //!< match resource active on specified node
    pe_find_inactive = 0x010, //!< match resource not running anywhere
    pe_find_any      = 0x020, //!< match base name of any clone instance
};

#  define pe_flag_have_quorum           0x00000001ULL
#  define pe_flag_symmetric_cluster     0x00000002ULL
#  define pe_flag_maintenance_mode      0x00000008ULL

#  define pe_flag_stonith_enabled       0x00000010ULL
#  define pe_flag_have_stonith_resource 0x00000020ULL
#  define pe_flag_enable_unfencing      0x00000040ULL
#  define pe_flag_concurrent_fencing    0x00000080ULL

#  define pe_flag_stop_rsc_orphans      0x00000100ULL
#  define pe_flag_stop_action_orphans   0x00000200ULL
#  define pe_flag_stop_everything       0x00000400ULL

#  define pe_flag_start_failure_fatal   0x00001000ULL
#  define pe_flag_remove_after_stop     0x00002000ULL
#  define pe_flag_startup_fencing       0x00004000ULL

#  define pe_flag_startup_probes        0x00010000ULL
#  define pe_flag_have_status           0x00020000ULL
#  define pe_flag_have_remote_nodes     0x00040000ULL

#  define pe_flag_quick_location        0x00100000ULL
#  define pe_flag_sanitized             0x00200000ULL
#  define pe_flag_stdout                0x00400000ULL

struct pe_working_set_s {
    xmlNode *input;
    crm_time_t *now;

    /* options extracted from the input */
    char *dc_uuid;
    pe_node_t *dc_node;
    const char *stonith_action;
    const char *placement_strategy;

    unsigned long long flags;

    int stonith_timeout;
    enum pe_quorum_policy no_quorum_policy;

    GHashTable *config_hash;
    GHashTable *tickets;

    // Actions for which there can be only one (e.g. fence nodeX)
    GHashTable *singletons;

    GListPtr nodes;
    GListPtr resources;
    GListPtr placement_constraints;
    GListPtr ordering_constraints;
    GListPtr colocation_constraints;
    GListPtr ticket_constraints;

    GListPtr actions;
    xmlNode *failed;
    xmlNode *op_defaults;
    xmlNode *rsc_defaults;

    /* stats */
    int num_synapse;
    int max_valid_nodes;
    int order_id;
    int action_id;

    /* final output */
    xmlNode *graph;

    GHashTable *template_rsc_sets;
    const char *localhost;
    GHashTable *tags;

    int blocked_resources;
    int disabled_resources;

    GList *param_check; // History entries that need to be checked
    GList *stop_needed; // Containers that need stop actions
};

enum pe_check_parameters {
    /* Clear fail count if parameters changed for un-expired start or monitor
     * last_failure.
     */
    pe_check_last_failure,

    /* Clear fail count if parameters changed for start, monitor, promote, or
     * migrate_from actions for active resources.
     */
    pe_check_active,
};

struct pe_node_shared_s {
    const char *id;
    const char *uname;
    enum node_type type;

    /* @TODO convert these flags into a bitfield */
    gboolean online;
    gboolean standby;
    gboolean standby_onfail;
    gboolean pending;
    gboolean unclean;
    gboolean unseen;
    gboolean shutdown;
    gboolean expected_up;
    gboolean is_dc;
    gboolean maintenance;
    gboolean rsc_discovery_enabled;
    gboolean remote_requires_reset;
    gboolean remote_was_fenced;
    gboolean remote_maintenance; /* what the remote-rsc is thinking */
    gboolean unpacked;

    int num_resources;
    pe_resource_t *remote_rsc;
    GListPtr running_rsc;       /* pe_resource_t* */
    GListPtr allocated_rsc;     /* pe_resource_t* */

    GHashTable *attrs;          /* char* => char* */
    GHashTable *utilization;
    GHashTable *digest_cache;   //!< cache of calculated resource digests
};

struct pe_node_s {
    int weight;
    gboolean fixed;
    int count;
    struct pe_node_shared_s *details;
    int rsc_discover_mode;
};

#  define pe_rsc_orphan                     0x00000001ULL
#  define pe_rsc_managed                    0x00000002ULL
#  define pe_rsc_block                      0x00000004ULL
#  define pe_rsc_orphan_container_filler    0x00000008ULL

#  define pe_rsc_notify                     0x00000010ULL
#  define pe_rsc_unique                     0x00000020ULL
#  define pe_rsc_fence_device               0x00000040ULL
#  define pe_rsc_promotable                 0x00000080ULL

#  define pe_rsc_provisional                0x00000100ULL
#  define pe_rsc_allocating                 0x00000200ULL
#  define pe_rsc_merging                    0x00000400ULL

#  define pe_rsc_reload                     0x00002000ULL
#  define pe_rsc_allow_remote_remotes       0x00004000ULL

#  define pe_rsc_failed                     0x00010000ULL
#  define pe_rsc_runnable                   0x00040000ULL
#  define pe_rsc_start_pending              0x00080000ULL

#  define pe_rsc_starting                   0x00100000ULL
#  define pe_rsc_stopping                   0x00200000ULL
#  define pe_rsc_allow_migrate              0x00800000ULL

#  define pe_rsc_failure_ignored            0x01000000ULL
#  define pe_rsc_maintenance                0x04000000ULL
#  define pe_rsc_is_container               0x08000000ULL

#  define pe_rsc_needs_quorum               0x10000000ULL
#  define pe_rsc_needs_fencing              0x20000000ULL
#  define pe_rsc_needs_unfencing            0x40000000ULL

enum pe_graph_flags {
    pe_graph_none = 0x00000,
    pe_graph_updated_first = 0x00001,
    pe_graph_updated_then = 0x00002,
    pe_graph_disable = 0x00004,
};

/* *INDENT-OFF* */
enum pe_action_flags {
    pe_action_pseudo = 0x00001,
    pe_action_runnable = 0x00002,
    pe_action_optional = 0x00004,
    pe_action_print_always = 0x00008,

    pe_action_have_node_attrs = 0x00010,
    pe_action_implied_by_stonith = 0x00040,
    pe_action_migrate_runnable =   0x00080,

    pe_action_dumped = 0x00100,
    pe_action_processed = 0x00200,
    pe_action_clear = 0x00400,
    pe_action_dangle = 0x00800,

    /* This action requires one or more of its dependencies to be runnable.
     * We use this to clear the runnable flag before checking dependencies.
     */
    pe_action_requires_any = 0x01000,

    pe_action_reschedule = 0x02000,
    pe_action_tracking = 0x04000,
};
/* *INDENT-ON* */

struct pe_resource_s {
    char *id;
    char *clone_name;
    xmlNode *xml;
    xmlNode *orig_xml;
    xmlNode *ops_xml;

    pe_working_set_t *cluster;
    pe_resource_t *parent;

    enum pe_obj_types variant;
    void *variant_opaque;
    resource_object_functions_t *fns;
    resource_alloc_functions_t *cmds;

    enum rsc_recovery_type recovery_type;

    // @TODO only pe_restart_restart is of interest, so merge into flags
    enum pe_restart restart_type; //!< \deprecated will be removed in future release

    int priority;
    int stickiness;
    int sort_index;
    int failure_timeout;
    int migration_threshold;
    guint remote_reconnect_ms;
    char *pending_task;

    unsigned long long flags;

    // @TODO merge these into flags
    gboolean is_remote_node;
    gboolean exclusive_discover;

    //!@{
    //! This field should be treated as internal to Pacemaker
    GListPtr rsc_cons_lhs;      // List of rsc_colocation_t*
    GListPtr rsc_cons;          // List of rsc_colocation_t*
    GListPtr rsc_location;      // List of pe__location_t*
    GListPtr actions;           // List of pe_action_t*
    GListPtr rsc_tickets;       // List of rsc_ticket*
    //!@}

    pe_node_t *allocated_to;
    pe_node_t *partial_migration_target;
    pe_node_t *partial_migration_source;
    GListPtr running_on;        /* pe_node_t*   */
    GHashTable *known_on;       /* pe_node_t*   */
    GHashTable *allowed_nodes;  /* pe_node_t*   */

    enum rsc_role_e role;
    enum rsc_role_e next_role;

    GHashTable *meta;
    GHashTable *parameters;
    GHashTable *utilization;

    GListPtr children;          /* pe_resource_t*   */
    GListPtr dangling_migrations;       /* pe_node_t*       */

    pe_resource_t *container;
    GListPtr fillers;

    pe_node_t *pending_node;    // Node on which pending_task is happening

#if ENABLE_VERSIONED_ATTRS
    xmlNode *versioned_parameters;
#endif
};

#if ENABLE_VERSIONED_ATTRS
// Used as action->action_details if action->rsc is not NULL
typedef struct pe_rsc_action_details_s {
    xmlNode *versioned_parameters;
    xmlNode *versioned_meta;
} pe_rsc_action_details_t;
#endif

struct pe_action_s {
    int id;
    int priority;

    pe_resource_t *rsc;
    pe_node_t *node;
    xmlNode *op_entry;

    char *task;
    char *uuid;
    char *cancel_task;
    char *reason;

    enum pe_action_flags flags;
    enum rsc_start_requirement needs;
    enum action_fail_response on_fail;
    enum rsc_role_e fail_role;

    GHashTable *meta;
    GHashTable *extra;

    /* 
     * These two varables are associated with the constraint logic
     * that involves first having one or more actions runnable before
     * then allowing this action to execute.
     *
     * These varables are used with features such as 'clone-min' which
     * requires at minimum X number of cloned instances to be running
     * before an order dependency can run. Another option that uses
     * this is 'require-all=false' in ordering constrants. This option
     * says "only require one instance of a resource to start before
     * allowing dependencies to start" -- basically, require-all=false is
     * the same as clone-min=1.
     */

    /* current number of known runnable actions in the before list. */
    int runnable_before;
    /* the number of "before" runnable actions required for this action
     * to be considered runnable */ 
    int required_runnable_before;

    GListPtr actions_before;    /* pe_action_wrapper_t* */
    GListPtr actions_after;     /* pe_action_wrapper_t* */

    /* Some of the above fields could be moved to the details,
     * except for API backward compatibility.
     */
    void *action_details; // varies by type of action
};

typedef struct pe_ticket_s {
    char *id;
    gboolean granted;
    time_t last_granted;
    gboolean standby;
    GHashTable *state;
} pe_ticket_t;

typedef struct pe_tag_s {
    char *id;
    GListPtr refs;
} pe_tag_t;

enum pe_link_state {
    pe_link_not_dumped,
    pe_link_dumped,
    pe_link_dup,
};

enum pe_discover_e {
    pe_discover_always = 0,
    pe_discover_never,
    pe_discover_exclusive,
};

/* *INDENT-OFF* */
enum pe_ordering {
    pe_order_none                  = 0x0,       /* deleted */
    pe_order_optional              = 0x1,       /* pure ordering, nothing implied */
    pe_order_apply_first_non_migratable = 0x2,  /* Only apply this constraint's ordering if first is not migratable. */

    pe_order_implies_first         = 0x10,      /* If 'then' is required, ensure 'first' is too */
    pe_order_implies_then          = 0x20,      /* If 'first' is required, ensure 'then' is too */
    pe_order_implies_first_master  = 0x40,      /* Imply 'first' is required when 'then' is required and then's rsc holds Master role. */

    /* first requires then to be both runnable and migrate runnable. */
    pe_order_implies_first_migratable  = 0x80,

    pe_order_runnable_left         = 0x100,     /* 'then' requires 'first' to be runnable */

    pe_order_pseudo_left           = 0x200,     /* 'then' can only be pseudo if 'first' is runnable */
    pe_order_implies_then_on_node  = 0x400,     /* If 'first' is required on 'nodeX',
                                                 * ensure instances of 'then' on 'nodeX' are too.
                                                 * Only really useful if 'then' is a clone and 'first' is not
                                                 */
    pe_order_probe                 = 0x800,     /* If 'first->rsc' is
                                                 *  - running but about to stop, ignore the constraint
                                                 *  - otherwise, behave as runnable_left
                                                 */

    pe_order_restart               = 0x1000,    /* 'then' is runnable if 'first' is optional or runnable */
    pe_order_stonith_stop          = 0x2000,    /* only applies if the action is non-pseudo */
    pe_order_serialize_only        = 0x4000,    /* serialize */
    pe_order_same_node             = 0x8000,    /* applies only if 'first' and 'then' are on same node */

    pe_order_implies_first_printed = 0x10000,   /* Like ..implies_first but only ensures 'first' is printed, not mandatory */
    pe_order_implies_then_printed  = 0x20000,   /* Like ..implies_then but only ensures 'then' is printed, not mandatory */

    pe_order_asymmetrical          = 0x100000,  /* Indicates asymmetrical one way ordering constraint. */
    pe_order_load                  = 0x200000,  /* Only relevant if... */
    pe_order_one_or_more           = 0x400000,  /* 'then' is runnable only if one or more of its dependencies are too */
    pe_order_anti_colocation       = 0x800000,

    pe_order_preserve              = 0x1000000, /* Hack for breaking user ordering constraints with container resources */
    pe_order_then_cancels_first    = 0x2000000, // if 'then' becomes required, 'first' becomes optional
    pe_order_trace                 = 0x4000000, /* test marker */
};
/* *INDENT-ON* */

typedef struct pe_action_wrapper_s {
    enum pe_ordering type;
    enum pe_link_state state;
    pe_action_t *action;
} pe_action_wrapper_t;

const char *rsc_printable_id(pe_resource_t *rsc);
gboolean cluster_status(pe_working_set_t * data_set);
pe_working_set_t *pe_new_working_set(void);
void pe_free_working_set(pe_working_set_t *data_set);
void set_working_set_defaults(pe_working_set_t * data_set);
void cleanup_calculations(pe_working_set_t * data_set);
void pe_reset_working_set(pe_working_set_t *data_set);
pe_resource_t *pe_find_resource(GListPtr rsc_list, const char *id_rh);
pe_resource_t *pe_find_resource_with_flags(GListPtr rsc_list, const char *id, enum pe_find flags);
pe_node_t *pe_find_node(GListPtr node_list, const char *uname);
pe_node_t *pe_find_node_id(GListPtr node_list, const char *id);
pe_node_t *pe_find_node_any(GListPtr node_list, const char *id, const char *uname);
GListPtr find_operations(const char *rsc, const char *node, gboolean active_filter,
                         pe_working_set_t * data_set);
int pe_bundle_replicas(const pe_resource_t *rsc);
#if ENABLE_VERSIONED_ATTRS
pe_rsc_action_details_t *pe_rsc_action_details(pe_action_t *action);
#endif

/*!
 * \brief Check whether a resource is any clone type
 *
 * \param[in] rsc  Resource to check
 *
 * \return TRUE if resource is clone, FALSE otherwise
 */
static inline bool
pe_rsc_is_clone(pe_resource_t *rsc)
{
    return rsc && (rsc->variant == pe_clone);
}

/*!
 * \brief Check whether a resource is a globally unique clone
 *
 * \param[in] rsc  Resource to check
 *
 * \return TRUE if resource is unique clone, FALSE otherwise
 */
static inline bool
pe_rsc_is_unique_clone(pe_resource_t *rsc)
{
    return pe_rsc_is_clone(rsc) && is_set(rsc->flags, pe_rsc_unique);
}

/*!
 * \brief Check whether a resource is an anonymous clone
 *
 * \param[in] rsc  Resource to check
 *
 * \return TRUE if resource is anonymous clone, FALSE otherwise
 */
static inline bool
pe_rsc_is_anon_clone(pe_resource_t *rsc)
{
    return pe_rsc_is_clone(rsc) && is_not_set(rsc->flags, pe_rsc_unique);
}

/*!
 * \brief Check whether a resource is part of a bundle
 *
 * \param[in] rsc  Resource to check
 *
 * \return TRUE if resource is part of a bundle, FALSE otherwise
 */
static inline bool
pe_rsc_is_bundled(pe_resource_t *rsc)
{
    return uber_parent(rsc)->parent != NULL;
}

// Deprecated type aliases
typedef struct pe_action_s action_t;                 //!< \deprecated Use pe_action_t instead
typedef struct pe_action_wrapper_s action_wrapper_t; //!< \deprecated Use pe_action_wrapper_t instead
typedef struct pe_node_s node_t;                     //!< \deprecated Use pe_node_t instead
typedef struct pe_resource_s resource_t;             //!< \deprecated Use pe_resource_t instead
typedef struct pe_tag_s tag_t;                       //!< \deprecated Use pe_tag_t instead
typedef struct pe_ticket_s ticket_t;                 //!< \deprecated Use pe_ticket_t instead
typedef enum pe_quorum_policy no_quorum_policy_t;    //!< \deprecated Use enum pe_quorum_policy instead

#ifdef __cplusplus
}
#endif

#endif
