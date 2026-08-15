//
//  EndpointResolver.swift
//  SpotiLEDKit
//
//  Created by wkuki on 15/08/2026.
//

import Foundation
import Network

/// Resolves a possibly-unresolved `NWEndpoint` (e.g. a Bonjour `.service`) to a concrete
/// `.url` endpoint carrying the given path, since `NWProtocolWebSocket` only sends a custom
/// request path for `.url` endpoints — `.service`/`.hostPort` endpoints always get "GET /".
final class EndpointResolver: @unchecked Sendable {
    private static let resolveTimeout: TimeInterval = 5
    private static let maxAttempts = 3
    private static let retryDelay: TimeInterval = 1

    private var connection: NWConnection?
    private var generation = 0

    /// Calls `completion` with the resolved `.url` endpoint, or `nil` if resolution failed —
    /// callers must treat `nil` as a connection failure rather than falling back to the
    /// unresolved endpoint, since that would silently connect to "/" instead of `path`.
    func resolve(_ endpoint: NWEndpoint, path: String, completion: @escaping (NWEndpoint?) -> Void) {
        resolveAddress(endpoint) { resolved in
            completion(resolved.flatMap { Self.webSocketEndpoint(from: $0, path: path) })
        }
    }

    /// Resolves any `NWEndpoint` (e.g. a Bonjour `.service`) down to the dialable host string a
    /// third party (like the controller, which isn't on this device and can't reuse our local
    /// Bonjour resolution) can use to reach it directly, or `nil` if resolution failed.
    func resolveHostString(_ endpoint: NWEndpoint, completion: @escaping (String?) -> Void) {
        resolveAddress(endpoint) { resolved in
            guard case .hostPort(let host, _) = resolved else {
                completion(nil)
                return
            }
            completion(Self.hostString(from: host))
        }
    }

    private func resolveAddress(_ endpoint: NWEndpoint, completion: @escaping (NWEndpoint?) -> Void) {
        generation += 1
        attemptResolve(endpoint, generation: generation, attempt: 1, completion: completion)
    }

    private func attemptResolve(_ endpoint: NWEndpoint, generation: Int, attempt: Int, completion: @escaping (NWEndpoint?) -> Void) {
        resolveOnce(endpoint) { [weak self] resolved in
            guard let self, generation == self.generation else { return }
            if let resolved {
                completion(resolved)
            } else if attempt < Self.maxAttempts {
                DispatchQueue.main.asyncAfter(deadline: .now() + Self.retryDelay) { [weak self] in
                    guard let self, generation == self.generation else { return }
                    self.attemptResolve(endpoint, generation: generation, attempt: attempt + 1, completion: completion)
                }
            } else {
                completion(nil)
            }
        }
    }

    private func resolveOnce(_ endpoint: NWEndpoint, completion: @escaping (NWEndpoint?) -> Void) {
        // Uses `.udp` so this is connectionless resolution only — a `.tcp` resolver would complete a real
        // handshake against the actual TCP listener and then abort it, which can break servers (e.g. one
        // that calls accept_async on the raw accepted stream and errors the whole accept loop on failure).
        let resolver = NWConnection(to: endpoint, using: .udp)
        connection = resolver

        var didComplete = false
        let finish: (NWEndpoint?) -> Void = { [weak self, weak resolver] resolved in
            guard !didComplete else { return }
            didComplete = true
            resolver?.cancel()
            if let self, let resolver, self.connection === resolver {
                self.connection = nil
            }
            completion(resolved)
        }

        resolver.stateUpdateHandler = { [weak resolver] state in
            switch state {
            case .ready:
                finish(resolver?.currentPath?.remoteEndpoint)
            case .failed, .cancelled:
                finish(nil)
            default:
                break
            }
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + Self.resolveTimeout) {
            finish(nil)
        }
        resolver.start(queue: .main)
    }

    func cancel() {
        generation += 1
        connection?.cancel()
        connection = nil
    }

    private static func webSocketEndpoint(from endpoint: NWEndpoint, path: String) -> NWEndpoint? {
        guard case .hostPort(let host, let port) = endpoint, let url = webSocketURL(host: host, port: port, path: path) else {
            return nil
        }
        return .url(url)
    }

    private static func webSocketURL(host: NWEndpoint.Host, port: NWEndpoint.Port, path: String) -> URL? {
        guard let hostString = hostString(from: host) else { return nil }
        return URL(string: "ws://\(hostString):\(port.rawValue)\(path)")
    }

    private static func hostString(from host: NWEndpoint.Host) -> String? {
        switch host {
        case .ipv4(let address):
            return stripInterfaceSuffix(address.debugDescription)
        case .ipv6(let address):
            return "[\(stripInterfaceSuffix(address.debugDescription))]"
        case .name(let name, _):
            return name
        @unknown default:
            return nil
        }
    }

    private static func stripInterfaceSuffix(_ description: String) -> String {
        String(description.split(separator: "%", maxSplits: 1).first ?? Substring(description))
    }
}
