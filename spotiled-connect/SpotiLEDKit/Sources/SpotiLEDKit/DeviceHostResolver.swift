//
//  DeviceHostResolver.swift
//  SpotiLEDKit
//
//  Created by wkuki on 15/08/2026.
//

import Network

/// Resolves a discovered device's `NWEndpoint` to a plain host string that a third party which
/// isn't on this device — like the controller — can dial directly, since it can't reuse the
/// Bonjour resolution this app already did locally.
public struct DeviceHostResolver: Sendable {
    private let resolver = EndpointResolver()

    public init() {}

    public func resolveHost(for endpoint: NWEndpoint) async -> String? {
        await withCheckedContinuation { continuation in
            resolver.resolveHostString(endpoint) { host in
                continuation.resume(returning: host)
            }
        }
    }
}
