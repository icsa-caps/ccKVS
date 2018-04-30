# ccKVS
An RDMA skew-aware key-value store, which implements the [Scale-Out ccNUMA](https://dl.acm.org/citation.cfm?id=3190550 "Scale-Out ccNUMA paper") design, to exploit skew in order to increase performance of data-serving applications.

In particular ccKVS consists of: 
* **Symmetric Caching**: 
a distributed cache architecture which allows requests for the most popular items to be executed on all nodes.
* **Fully distributed strongly consistent protocols**: 
to efficiently handle writes while maintaining consistency of the caches.

We briefly explain these ideas bellow, more details can be found in our Eurosys'18 [paper](https://dl.acm.org/citation.cfm?id=3190550 "Scale-Out ccNUMA paper")  and [slides](https://www.slideshare.net/AntoniosKatsarakis/scaleout-ccnuma-eurosys18 "Scale-Out ccNUMA slides").

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
Protocols are implemented efficiently on top of RDMA, offering:
* **Fully distributed writes** 
    * Writes (for any key) are directly executed on any node, as oposed using a primary node --> hot-spot
    * Single global writes ordering is guaranteed via per-key logical (lamport) clocks
      * Reduces network RTTs
      * Avoids hot-spots and evenly spread the write propagation costs to all nodes
* Two per-key **strongly consistent** flavours:
    * **Linearizability** (Lin - strongest --> 2 network RTTs): 1) Broadcast Invalidations* 2) Broadcast Updates*
    * **Sequential Consistency** (SC --> 1RTT): 1) Broadcast Updates* 
    * *along with logical (Lamport) clocks

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

## Disclaimer
1. ccKVS is based on [HERD/MICA](https://github.com/efficient/rdma_bench/tree/master/herd "HERD repo") design as an underlying KVS, the code of which we have adapted to implement both our underlying KVS and our (symmetric) caches.
2. Similarly for implementing efficient (CRCW) synchronization over seqlocks we have used the [OPTIK](https://github.com/LPD-EPFL/ASCYLIB "OPTIK repo") library.

More details can be found in our Eurosys'18 [paper](https://dl.acm.org/citation.cfm?id=3190550 "Scale-Out ccNUMA paper")  and [slides](https://www.slideshare.net/AntoniosKatsarakis/scaleout-ccnuma-eurosys18 "Scale-Out ccNUMA slides").
