+-------------------------------------------------------------------+
| Tier 1: Kernel Bypass (DPDK / OpenOnload)                         |  10M+ RPS / core
| - Direct hardware ring buffers, zero context switches             |
+-------------------------------------------------------------------+
| Tier 2: Asynchronous Ring-Buffer I/O (Linux io_uring / Seastar)   |  1M - 5M RPS / core
| - Shared submission/completion queues, zero-copy socket I/O       |
+-------------------------------------------------------------------+
| Tier 3: Event-Driven Non-Blocking I/O (epoll / Boost.Asio)        |  500k - 1M RPS / core
| - Standard OS network stack, non-blocking sockets                 |
+-------------------------------------------------------------------+


┌────────────────────────────────────────────────────────────────────────┐
│                        Kubernetes API Server                           │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │ HTTP Chunked Stream (Watch)
                                   ▼
┌────────────────────────────────────────────────────────────────────────┐
│ 1. Reflector                                                           │
│    - Calls LIST to populate initial state                              │
│    - Calls WATCH with resourceVersion to stream delta events           │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │ WatchEvent { ADDED, MODIFIED, DELETED }
                                   ▼
┌────────────────────────────────────────────────────────────────────────┐
│ 2. DeltaFIFO Queue                                                     │
│    - Thread-safe queue buffering change deltas per object key          │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │ Pop()
                                   ▼
┌────────────────────────────────────────────────────────────────────────┐
│ 3. SharedInformer / Controller                                         │
│    - Updates local cache AND triggers registered C++ callbacks     │
└──────────────────┬──────────────────────────────────┬──────────────────┘
                   │                                  │
                   ▼                                  ▼
┌──────────────────────────────────────┐  ┌──────────────────────────────┐
│ 4. Indexer / ThreadSafeStore         │  │ 5. Event Handlers            │
│    - O(1) Local RAM Cache            │  │    - OnAdd, OnUpdate,        │
│    - Custom secondary indices        │  │      OnDelete callbacks      │
└──────────────────────────────────────┘  └──────────────────────────────┘