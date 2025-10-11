# 🛡️ Security Poster: Bulkhead Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Resource Exhaustion (CWE-770): One partition consumes all
├─ Partition Bypass: Attacker accesses wrong partition
├─ Deadlock: Cross-partition dependencies
└─ DoS: Flood one partition to impact service

🟡 ВЫСОКИЕ:
├─ Unbalanced Partitions: Some partitions idle, others overloaded
├─ Partition Leak: Resources not returned to partition
└─ Priority Inversion: Low-priority starves high-priority
```

## 💣 Ключевые уязвимости

### 1. Resource Exhaustion - Single Partition Overflow
```cpp
// ❌ УЯЗВИМО - No per-partition limits
class VulnerableBulkhead {
    ThreadPool shared_pool_;  // ⚠️ Shared!
    
public:
    void handle_request(Request req) {
        shared_pool_.submit([req] {
            process(req);  // 💥 One service can consume all threads!
        });
    }
};

// ✅ БЕЗОПАСНО - Isolated partitions
class SecureBulkhead {
    std::map<Service, ThreadPool> partitions_;
    
public:
    SecureBulkhead() {
        // ✅ Dedicated partition per service
        partitions_[SERVICE_A] = ThreadPool(10);  // 10 threads for A
        partitions_[SERVICE_B] = ThreadPool(10);  // 10 threads for B
        partitions_[SERVICE_C] = ThreadPool(5);   // 5 threads for C
    }
    
    void handle_request(Service svc, Request req) {
        auto& pool = partitions_.at(svc);
        
        if (!pool.try_submit(req, timeout=100ms)) {
            throw PartitionFullException();  // ✅ Reject, don't wait
        }
    }
};
```

### 2. Partition Bypass Attack
```cpp
// ❌ УЯЗВИМО - User controls partition selection
void handle_request(const Request& req) {
    Service svc = req.get_service();  // ⚠️ From user input!
    bulkhead.execute(svc, req.get_task());  // 💥 Bypass!
}

// Attacker sends: service="ADMIN" in request
// → Uses ADMIN partition instead of USER partition

// ✅ БЕЗОПАСНО - Server-side partition selection
void handle_request(const Request& req) {
    // ✅ Determine service from authenticated context
    auto user = authenticate(req);
    Service svc = determine_service(user.get_role());
    
    bulkhead.execute(svc, req.get_task());
}
```

### 3. Cross-Partition Deadlock
```cpp
// ❌ DEADLOCK - Partitions waiting on each other
void service_a_task() {
    auto resource_b = partition_b.acquire();  // Waits for B
    process_with(resource_b);
}

void service_b_task() {
    auto resource_a = partition_a.acquire();  // Waits for A
    process_with(resource_a);  // 💥 Deadlock!
}

// ✅ БЕЗОПАСНО - Avoid cross-partition dependencies
void service_a_task() {
    // ✅ Use only resources from own partition
    auto resource_a = partition_a.acquire();
    process_with(resource_a);
    
    // ✅ If need B, use async message passing
    send_async_request_to_service_b();
}
```

### 4. Partition Resource Leak
```cpp
// ❌ УТЕЧКА - Resources not returned
class LeakyPartition {
public:
    Resource* acquire() {
        if (available_.empty()) {
            throw NoResourceException();
        }
        auto* res = available_.back();
        available_.pop_back();
        return res;  // ⚠️ Caller forgets to return!
    }
    
    void release(Resource* res) {
        available_.push_back(res);
    }
};

// ✅ БЕЗОПАСНО - RAII guard
class PartitionGuard {
    Partition& partition_;
    Resource* resource_;
    
public:
    PartitionGuard(Partition& p) : partition_(p) {
        resource_ = partition_.acquire();
    }
    
    ~PartitionGuard() {
        partition_.release(resource_);  // ✅ Auto-return
    }
    
    Resource* get() { return resource_; }
};

// Использование:
{
    PartitionGuard guard(partition);
    use_resource(guard.get());
}  // ✅ Auto-released
```

## 🛠️ Инструменты
```bash
# Partition utilization monitoring
./monitor_bulkhead --report-per-partition

# Deadlock detection
valgrind --tool=helgrind ./bulkhead_app

# Resource leak detection
valgrind --leak-check=full ./bulkhead_app
```

## 📋 Checklist
```
[ ] Dedicated resource pools per partition
[ ] Server-side partition selection
[ ] No cross-partition dependencies
[ ] RAII для resource management
[ ] Per-partition metrics (utilization, queue size)
[ ] Rejection policy при full partition
[ ] Dynamic partition resizing (optional)
[ ] Circuit breakers per partition
```

## 💡 Золотое правило
```
"Bulkhead isolates failures - DON'T share resources.
 Server decides partition, not user.
 No cross-partition waits (deadlock risk).
 RAII для resource lifecycle!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

