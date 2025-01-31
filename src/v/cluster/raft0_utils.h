/*
 * Copyright 2020 Redpanda Data, Inc.
 *
 * Use of this software is governed by the Business Source License
 * included in the file licenses/BSL.md
 *
 * As of the Change Date specified in that file, in accordance with
 * the Business Source License, use of this software will be governed
 * by the Apache License, Version 2.0
 */

#pragma once

#include "cluster/controller_stm.h"
#include "cluster/logger.h"
#include "cluster/partition_manager.h"
#include "cluster/shard_table.h"
#include "model/fundamental.h"
#include "model/namespace.h"
#include "raft/fundamental.h"

#include <vector>
namespace cluster {

static ss::future<consensus_ptr> create_raft0(
  ss::sharded<partition_manager>& pm,
  ss::sharded<shard_table>& st,
  const ss::sstring& data_directory,
  const std::vector<model::node_id>& initial_nodes) {
    if (!initial_nodes.empty()) {
        vlog(clusterlog.info, "Current node is a cluster founder");
    }
    // controller log size is maintained with controller snapshots
    auto overrides = std::make_unique<storage::ntp_config::default_overrides>();
    overrides->cleanup_policy_bitflags = model::cleanup_policy_bitflags::none;

    std::vector<raft::vnode> initial_vnodes;
    initial_vnodes.reserve(initial_nodes.size());
    for (auto& id : initial_nodes) {
        initial_vnodes.emplace_back(id, model::revision_id(0));
    }

    return pm.local()
      .manage(
        storage::ntp_config(
          model::controller_ntp, data_directory, std::move(overrides)),
        raft::group_id(0),
        std::move(initial_vnodes),
        raft::with_learner_recovery_throttle::no,
        raft::keep_snapshotted_log::no,
        std::nullopt)
      .then([&st](consensus_ptr p) {
          // Add raft 0 to shard table
          return st
            .invoke_on_all([gr = p->group()](shard_table& local_st) {
                local_st.update(
                  model::controller_ntp,
                  gr,
                  controller_stm_shard,
                  model::revision_id(0));
            })
            .then([p] { return p; });
      });
}
} // namespace cluster
