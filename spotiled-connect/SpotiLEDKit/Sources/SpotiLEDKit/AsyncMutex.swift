//
//  AsyncMutex.swift
//  SpotiLEDKit
//

import Foundation

/// Serializes async work so at most one `withLock` body runs at a time; concurrent callers
/// queue in FIFO order instead of running interleaved.
final class AsyncMutex: @unchecked Sendable {
    private let lock = NSLock()
    private var isLocked = false
    private var waiters: [CheckedContinuation<Void, Never>] = []

    func withLock<T>(_ body: () async throws -> T) async rethrows -> T {
        await acquire()
        defer { release() }
        return try await body()
    }

    private func acquire() async {
        await withCheckedContinuation { continuation in
            if acquireOrEnqueue(continuation) {
                continuation.resume()
            }
        }
    }

    /// Atomically checks and updates lock state under `lock`. Returns whether the caller may
    /// proceed immediately (`true`) or was queued to be resumed later by `release` (`false`).
    private func acquireOrEnqueue(_ continuation: CheckedContinuation<Void, Never>) -> Bool {
        lock.lock()
        defer { lock.unlock() }
        if isLocked {
            waiters.append(continuation)
            return false
        }
        isLocked = true
        return true
    }

    private func release() {
        lock.lock()
        if waiters.isEmpty {
            isLocked = false
            lock.unlock()
        } else {
            let next = waiters.removeFirst()
            lock.unlock()
            next.resume()
        }
    }
}
