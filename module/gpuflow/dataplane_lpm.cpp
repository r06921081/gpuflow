/*
 * Copyright 2017 of original authors and authors.
 *
 * We use MIT license for this project, checkout LICENSE file in the root of source tree.
 */

#include <cstdlib>
#include <iostream>
#include <rte_lpm.h>
#include <rte_ip.h>
#include <rte_lpm6.h>
#include "dataplane_lpm.h"

namespace gpuflow {

DataPlaneLPMv4::DataPlaneLPMv4() {
  for(uint8_t i = 0; i < 4; ++i) {
    // The routing table is located in, ip addr 10.(0-3).x.x, netmask 255.255.0.0
    ipv4_lpm_route_array[i].ip = IPv4(17, i, 0, 0);
    ipv4_lpm_route_array[i].depth = 16;
    ipv4_lpm_route_array[i].if_out = i;
  }
  CreateLPMTable(0);
}

void DataPlaneLPMv4::CreateLPMTable(int socket_id) {
  char table_name[64];
  snprintf(table_name, sizeof(table_name), "IPV4_L3FWD_LPM_%d", socket_id);
  struct rte_lpm_config config_ipv4;
  config_ipv4.max_rules = MAX_LPM_ROUTING_RULES;
  config_ipv4.number_tbl8s = (1 << 8);
  config_ipv4.flags = 0;
  ipv4_lpm_lookup_struct[socket_id] = rte_lpm_create(table_name, socket_id, &config_ipv4);
  if (ipv4_lpm_lookup_struct[socket_id] == nullptr) {
    std::cerr << "Can't create ipv4 lpm look up struct on socket id : " << socket_id << std::endl;
    exit(1);
  }
  int ret = 0;
  for(unsigned int i = 0; i < 4; ++i) {
    ret = rte_lpm_add(ipv4_lpm_lookup_struct[socket_id], ipv4_lpm_route_array[i].ip,
                      ipv4_lpm_route_array[i].depth, ipv4_lpm_route_array[i].if_out);
    if (ret < 0) {
      std::cerr << "Error occurred on add lpm, abort" << std::endl;
      exit(1);
    }
  }
}

uint16_t DataPlaneLPMv4::RoutingTableLookUp(ipv4_hdr *ipv4_header, uint16_t port_id, int socket_id){
  unsigned int next_hop;
  return (uint16_t ) ((rte_lpm_lookup(ipv4_lpm_lookup_struct[socket_id],
                                      rte_be_to_cpu_32(ipv4_header->dst_addr),
                                      &next_hop) == 0)? next_hop : port_id);
}

DataPlaneLPMv6::DataPlaneLPMv6() {
  for(uint8_t i = 0; i < 4; ++i) {
    // The routing table is located in, ip addr 10.(0-3).x.x, netmask 255.255.0.0
    ipv6_lpm_route_array[i].ip[0] = (uint8_t) 10;
    ipv6_lpm_route_array[i].ip[1] = i;
    for(int cls = 2; cls < 16; ++cls) {
      ipv6_lpm_route_array[i].ip[cls] = 0;
    }
    ipv6_lpm_route_array[i].depth = 64;
    ipv6_lpm_route_array[i].if_out = i;
  }
  CreateLPMTable(0);
}

void DataPlaneLPMv6::CreateLPMTable(int socket_id) {
  char table_name[64];
  snprintf(table_name, sizeof(table_name), "IPV6_L3FWD_LPM_%d", socket_id);
  struct rte_lpm6_config config_ipv6;
  config_ipv6.max_rules = MAX_LPM_ROUTING_RULES;
  config_ipv6.number_tbl8s = (1 << 8);
  config_ipv6.flags = 0;
  ipv6_lpm_lookup_struct[socket_id] = rte_lpm6_create(table_name, socket_id, &config_ipv6);
  if (ipv6_lpm_lookup_struct[socket_id] == nullptr) {
    std::cerr << "Can't create ipv4 lpm look up struct on socket id : " << socket_id << std::endl;
    exit(1);
  }
  for(unsigned int i = 0; i < 4; ++i) {
    int ret = rte_lpm6_add(ipv6_lpm_lookup_struct[socket_id], ipv6_lpm_route_array[i].ip,
                      ipv6_lpm_route_array[i].depth, ipv6_lpm_route_array[i].if_out);
    if (ret < 0) {
      std::cerr << "Error occurred on add lpm, abort" << std::endl;
      exit(1);
    }
  }
}

uint16_t DataPlaneLPMv6::RoutingTableLookUp(ipv6_hdr *ipv6_header, uint16_t port_id, int socket_id){
  unsigned int next_hop;
  return (uint16_t ) ((rte_lpm6_lookup(ipv6_lpm_lookup_struct[socket_id],
                                       ipv6_header->dst_addr,
                                      &next_hop) == 0)? next_hop : port_id);
}

} // namespace gpuflow