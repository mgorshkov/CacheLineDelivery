//
// Created by Mikhail Gorshkov on 29.04.2022.
//

#pragma once

#include <pthread.h>
#include <atomic>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

class SpinLock {
public:
    inline explicit SpinLock(bool isPrivate = true) {
        int rc = pthread_spin_init(&m_lock, isPrivate ? PTHREAD_PROCESS_PRIVATE : PTHREAD_PROCESS_SHARED);
        if (unlikely(rc != 0)) {
            std::cerr << "Error calling pthread_spin_init: " << rc << std::endl;
        }
    }

    inline ~SpinLock() {
        int rc = pthread_spin_destroy(&m_lock);
        if (unlikely(rc != 0)) {
            std::cerr << "Error calling pthread_spin_destroy: " << rc << std::endl;
        }
    }

    inline void lock() {
        int rc = pthread_spin_lock(&m_lock);
        if (unlikely(rc != 0)) {
            std::cerr << "Error calling pthread_spin_lock: " << rc << std::endl;
        }
    }

    inline void unlock() {
        int rc = pthread_spin_unlock(&m_lock);
        if (unlikely(rc != 0)) {
            std::cerr << "Error calling pthread_spin_unlock: " << rc << std::endl;
        }
    }

private:
    pthread_spinlock_t m_lock;
};

class AtomicFlagLock {
public:
    inline void lock() {
        while (m_lock.test_and_set(std::memory_order_acquire));
    }

    inline void unlock() {
        m_lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

template <typename Lock>
class Locker {
public:
    inline explicit Locker(Lock& lock)
            : m_lock{lock}{
        m_lock.lock();
    }

    inline ~Locker() {
        m_lock.unlock();
    }

private:
    Lock& m_lock;
};
