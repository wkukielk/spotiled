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
final class EndpointResolver {
    private var connection: NWConnection?

    /// Calls `completion` with the resolved `.url` endpoint, or `nil` if resolution failed —
    /// callers must treat `nil` as a connection failure rather than falling back to the
    /// unresolved endpoint, since that would silently connect to "/" instead of `path`.
    func resolve(_ endpoint: NWEndpoint, path: String, completion: @escaping (NWEndpoint?) -> Void) {
        // Uses `.udp` so this is connectionless resolution only — a `.tcp` resolver would complete a real
        // handshake against the actual TCP listener and then abort it, which can break servers (e.g. one
        // that calls accept_async on the raw accepted stream and errors the whole accept loop on failure).
        let resolver = NWConnection(to: endpoint, using: .udp)
        connection = resolver
        resolver.stateUpdateHandler = { [weak self, weak resolver] state in
            guard let self, let resolver else { return }
            switch state {
            case .ready:
                let resolved = resolver.currentPath?.remoteEndpoint
                resolver.cancel()
                self.connection = nil
                completion(resolved.flatMap { Self.webSocketEndpoint(from: $0, path: path) })
            case .failed, .cancelled:
                resolver.cancel()
                self.connection = nil
                completion(nil)
            default:
                break
            }
        }
        resolver.start(queue: .main)
    }

    func cancel() {
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
        let hostString: String
        switch host {
        case .ipv4(let address):
            hostString = stripInterfaceSuffix(address.debugDescription)
        case .ipv6(let address):
            hostString = "[\(stripInterfaceSuffix(address.debugDescription))]"
        case .name(let name, _):
            hostString = name
        @unknown default:
            return nil
        }
        return URL(string: "ws://\(hostString):\(port.rawValue)\(path)")
    }

    private static func stripInterfaceSuffix(_ description: String) -> String {
        String(description.split(separator: "%", maxSplits: 1).first ?? Substring(description))
    }
}
