# ccKVS
An RDMA skew-aware key-value store, which implements the "Scale-Out ccNUMA" design, to exploit skew in order to increase performance of data-serving applications.

In particular ccKVS consists of: 
* **Symmetric Caching**: 
a distributed cache architecture which allows requests for the most popular items to be executed on all nodes.
* **Fully distributed strongly consistent protocols**: 
to efficiently handle writes while maintaining consistency of the caches.

## Symmetric Caching
* Every node contains an identical cache storing the hottest keys in the cluster
* Uniformly spread the requests to all of the nodes
  * Requests for the hottests objects (majority) will be served on all nodes locally
  * Requests missing the cache will either be served through the local portion of KVS or more likely through the RDMA network
* **Benefits**:
  * **Load balances** and **filters the skew**
  * **Throughput scales** with the number of servers
  * **Less network b/w** due to requests served by caches

## Fully distributed strongly consistent protocols
* protocols implemented efficiently on top of RDMA
* offering **fully distributed writes** 
    * Writes (for any key) are directly executed on any node, as oposed using a primary node --> hot-spot
    * single global writes ordering is guaranteed via per-key logical (lamport) clocks
      * Reduces network RTTs
      * Avoids hot-spots and evenly spread the write propagation costs to all nodes
* Two per-key **strongly consistent** flavours:
    * **Linearizability** (Lin - strongest --> 2 network RTTs): Invalidate & Update caches
    * **Sequential Consistency** (SC - 1RTT): Update the caches
    
## Repository Contains
1. ccKVS is based on HERD/MICA design as an underlying KVS, the code of which we have modified to implement both our underlying KVS and our (symmetric) caches.
2. Similarly for implementing efficient (CRCW) synchronization over seqlocks we have used the optiks library.
More details can be found in our Eurosys'18 paper.

## Requirments

### Dependencies
1. numactl
1. libgsl0-dev
1. libnuma-dev
1. libatmomic_ops
1. libmemcached-dev
1. MLNX_OFED_LINUX-4.1-1.0.2.0

### Settings
1. Run subnet-manager in one of the nodes: '/etc/init.d/opensmd start'
1. On every node apply the following:
 1. echo 8192 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
 1. echo 8192 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages  
 1. echo 10000000001 > /proc/sys/kernel/shmmax
 1. echo 10000000001 > /proc/sys/kernel/shmall
 1. Make sure that the changes have been applied using cat on the above files
 1. The following changes are temporary (i.e. need to be performed after a reboot)

## Tested on
* Infiniband cluster of 9 inter-connected nodes, via a Mellanox MSX6012F-BS switch, each one equiped with a single-port 56Gb Infiniband NIC (Mellanox MCX455A-FCAT PCIe-gen3 x16).
* OS: Ubuntu 14.04 (Kernel: 3.13.0-32-generic) 
