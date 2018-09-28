/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/bpf.h>
#include "bpf_kern.h"


ELF_SEC(BPF_CGROUP_INGRESS_PROG_NAME)
int bpf_cgroup_ingress(struct __sk_buff* skb) {
    return bpf_traffic_account(skb, BPF_INGRESS);
}

ELF_SEC(BPF_CGROUP_EGRESS_PROG_NAME)
int bpf_cgroup_egress(struct __sk_buff* skb) {
    return bpf_traffic_account(skb, BPF_EGRESS);
}

ELF_SEC(XT_BPF_EGRESS_PROG_NAME)
int xt_bpf_egress_prog(struct __sk_buff* skb) {
    uint32_t key = skb->ifindex;
    bpf_update_stats(skb, IFACE_STATS_MAP, BPF_EGRESS, &key);
    return BPF_MATCH;
}

ELF_SEC(XT_BPF_INGRESS_PROG_NAME)
int xt_bpf_ingress_prog(struct __sk_buff* skb) {
    uint32_t key = skb->ifindex;
    bpf_update_stats(skb, IFACE_STATS_MAP, BPF_INGRESS, &key);
    return BPF_MATCH;
}

ELF_SEC(XT_BPF_WHITELIST_PROG_NAME)
int xt_bpf_whitelist_prog(struct __sk_buff* skb) {
    uint32_t sock_uid = get_socket_uid(skb);
    if (is_system_uid(sock_uid)) return BPF_MATCH;
    uint8_t* whitelistMatch = find_map_entry(UID_OWNER_MAP, &sock_uid);
    if (whitelistMatch) return *whitelistMatch & HAPPY_BOX_MATCH;
    return BPF_NOMATCH;
}

ELF_SEC(XT_BPF_BLACKLIST_PROG_NAME)
int xt_bpf_blacklist_prog(struct __sk_buff* skb) {
    uint32_t sock_uid = get_socket_uid(skb);
    uint8_t* blacklistMatch = find_map_entry(UID_OWNER_MAP, &sock_uid);
    if (blacklistMatch) return *blacklistMatch & PENALTY_BOX_MATCH;
    return BPF_NOMATCH;
}
