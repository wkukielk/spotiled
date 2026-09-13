//
//  ControllerTrustStore.swift
//  SpotiLEDKit
//
//  Created by wkuki on 13/09/2026.
//

import Foundation

/// Persists which controller certificates the user has verified via SAS pairing, keyed by the
/// controller's Bonjour name. The fingerprint is a public SHA-256 hash, not a secret, so
/// `UserDefaults` is sufficient — this doesn't need Keychain-level protection.
public struct ControllerTrustStore {
    private static let keyPrefix = "com.spotiled.controllerTrust."

    private let defaults: UserDefaults

    public init(defaults: UserDefaults = .standard) {
        self.defaults = defaults
    }

    public func pinnedFingerprint(for name: String) -> Data? {
        defaults.data(forKey: Self.key(for: name))
    }

    public func pin(_ fingerprint: Data, for name: String) {
        defaults.set(fingerprint, forKey: Self.key(for: name))
    }

    public func forget(_ name: String) {
        defaults.removeObject(forKey: Self.key(for: name))
    }

    private static func key(for name: String) -> String {
        keyPrefix + name
    }
}
