//
//  OneShotContinuation.swift
//  SpotiLEDKit
//

import Foundation

/// Bridges a callback-based API to `async`/`await`, resolving exactly once regardless of
/// whether `finish` is called before or after `wait` starts.
final class OneShotContinuation<Success: Sendable>: @unchecked Sendable {
    private let lock = NSLock()
    private var continuation: CheckedContinuation<Success, Error>?
    private var pendingResult: Result<Success, Error>?
    private var isFinished = false

    func wait() async throws -> Success {
        try await withCheckedThrowingContinuation { continuation in
            lock.lock()
            if let pendingResult {
                self.pendingResult = nil
                lock.unlock()
                continuation.resume(with: pendingResult)
                return
            }
            self.continuation = continuation
            lock.unlock()
        }
    }

    /// Resolves with `result`, unless a previous call already did so. Returns whether this call
    /// was the one that resolved it — useful for guarding one-time teardown alongside it.
    @discardableResult
    func finish(_ result: Result<Success, Error>) -> Bool {
        var continuationToResume: CheckedContinuation<Success, Error>?
        lock.lock()
        if isFinished {
            lock.unlock()
            return false
        }
        isFinished = true
        continuationToResume = continuation
        continuation = nil
        if continuationToResume == nil {
            pendingResult = result
        }
        lock.unlock()
        continuationToResume?.resume(with: result)
        return true
    }
}
