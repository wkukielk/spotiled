//
//  DeviceHostResolver.swift
//  SpotiLEDKit
//
//  Created by wkuki on 15/08/2026.
//

import Network

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
