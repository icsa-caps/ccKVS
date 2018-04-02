# ccKVS
A skew-aware key-value store build on top of RDMA, based on MICA key-value store and adapting HERD libraries, utilizing the techniques of symmetric caching to offer increase performance over load balancing.

## Symmetric Caching
Every node contains an identical popularity-based cache of keys stored in the system
* **Incremental Locking**: (Core Sharding + Caching) serves:
 * cold keys via a _single core_ (EREW), avoiding the cost of synchronization,
 * hot keys using  _multiple cores_ (CRCW) exploiting optimistic concurency control (lock-free reads) & _several machines_ (replication via caching)
* **Unified Protocols for Replication and Atomicity**:
 * distributed coherence protocols implemented on top of RDMA (offering per key Strong and Sequencial Consistency)

## Repository Contains
1. A modified version of HERD/MICA where each server acts both as client and a server over UD and UC transports.
2. Similar to 1. but using UD transports only
3. Armonia KVS built on top of 2. and using the technique of symetric caching to exploit skew for high performance.

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
 1. echo 8192 | tee /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages > /dev/null
 1. echo 10000000001 | tee /proc/sys/kernel/shmmax /proc/sys/kernel/shmall > /dev/null
 * Make sure that the changes have been applied using cat on the above files
 * The following changes are temporary (i.e. need to be performed after a reboot)

## Tested on
* Infiniband cluster of 9 inter-connected nodes, via a Mellanox MSX6012F-BS switch, each one equiped with a single-port 56Gb Infiniband NIC (Mellanox MCX455A-FCAT PCIe-gen3 x16).
* OS: Ubuntu 14.04 (Kernel: 3.13.0-32-generic) 
