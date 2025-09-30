// Yep, "Blue" means Nintendo Switch!

#pragma once

#include <nn/os.h>

#include <nn/diag/diag_Break.h>
#define JPH_BREAKPOINT NN_DIAG_BREAK()

#include <nn/os/os_TickTypes.h>
#define JPH_PLATFORM_BLUE_GET_TICKS() nn::os::GetSystemTick().GetInt64Value()

#include <nn/os/os_MutexTypes.h>
#define JPH_PLATFORM_BLUE_MUTEX nn::os::MutexType
#define JPH_PLATFORM_BLUE_MUTEX_INIT(mutex) nn::os::InitializeMutex(&mutex, false, 0)
#define JPH_PLATFORM_BLUE_MUTEX_DESTROY(mutex) nn::os::FinalizeMutex(&mutex)
#define JPH_PLATFORM_BLUE_MUTEX_TRYLOCK(mutex) nn::os::TryLockMutex(&mutex)
#define JPH_PLATFORM_BLUE_MUTEX_LOCK(mutex) nn::os::LockMutex(&mutex)
#define JPH_PLATFORM_BLUE_MUTEX_UNLOCK(mutex) nn::os::UnlockMutex(&mutex)

#include <nn/os/os_ReaderWriterLockTypes.h>
#define JPH_PLATFORM_BLUE_RWLOCK nn::os::ReaderWriterLockType
#define JPH_PLATFORM_BLUE_RWLOCK_INIT(lock) nn::os::InitializeReaderWriterLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_DESTROY(lock) nn::os::FinalizeReaderWriterLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_TRYWLOCK(lock) nn::os::TryAcquireWriteLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_TRYRLOCK(lock) nn::os::TryAcquireReadLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_WLOCK(lock) nn::os::AcquireWriteLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_RLOCK(lock) nn::os::AcquireReadLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_WUNLOCK(lock) nn::os::ReleaseWriteLock(&lock)
#define JPH_PLATFORM_BLUE_RWLOCK_RUNLOCK(lock) nn::os::ReleaseReadLock(&lock)

#define JPH_PLATFORM_BLUE_SEMAPHORE nn::os::SemaphoreType
#define JPH_PLATFORM_BLUE_SEMAPHORE_INIT(mSemaphore) nn::os::InitializeSemaphore(&mSemaphore, 0, INT_MAX);
#define JPH_PLATFORM_BLUE_SEMAPHORE_DESTROY(mSemaphore) nn::os::FinalizeSemaphore(&mSemaphore)
#define JPH_PLATFORM_BLUE_SEMAPHORE_SIGNAL(mSemaphore, num_to_acquire) nn::os::ReleaseSemaphore(&mSemaphore, num_to_release)
#define JPH_PLATFORM_BLUE_SEMAPHORE_WAIT(mSemaphore) nn::os::AcquireSemaphore(&mSemaphore);

