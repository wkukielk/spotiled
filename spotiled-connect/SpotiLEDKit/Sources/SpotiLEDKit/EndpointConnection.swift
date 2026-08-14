//
//  EndpointConnection.swift
//  SpotiLEDKit
//
//  Created by wkuki on 25/07/2026.
//

import Foundation
import Network
import Observation
import Mockable

public enum EndpointConnectionState: Equatable, Sendable {
    case disconnected
    case connecting(name: String)
    case connected(name: String)
    case failed(name: String)
}

public enum EndpointConnectionError: Error, Equatable {
    case notConnected
    case sendInProgress
    case connectionFailed
    case unexpectedMessage
}

@Mockable
public protocol EndpointConnecting: AnyObject {
    var state: EndpointConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String, path: String)
    func disconnect()
    func send(_ message: String) async throws -> String
    func sendOneWay(_ message: String) async throws
}

@Observable
public final class WebSocketEndpointConnection: EndpointConnecting, @unchecked Sendable {
    public private(set) var state: EndpointConnectionState = .disconnected

    private var connection: NWConnection?
    private var pendingReply: CheckedContinuation<String, Error>?

    private var resolver: NWConnection?

    public init() {}

    public func connect(to endpoint: NWEndpoint, name: String, path: String) {
        disconnect()
        state = .connecting(name: name)

        // NWProtocolWebSocket only sends a custom request path for `.url` endpoints; `.service`/`.hostPort`
        // endpoints always get "GET /". So resolve to a concrete host/port first, then reconnect via `.url`.
        // Uses `.udp` so this is connectionless resolution only — a `.tcp` resolver would complete a real
        // handshake against the actual TCP listener and then abort it, which can break servers (e.g. one
        // that calls accept_async on the raw accepted stream and errors the whole accept loop on failure).
        let resolver = NWConnection(to: endpoint, using: .udp)
        self.resolver = resolver
        resolver.stateUpdateHandler = { [weak self, weak resolver] resolverState in
            guard let self, let resolver else { return }
            switch resolverState {
            case .ready:
                let resolved = resolver.currentPath?.remoteEndpoint
                resolver.cancel()
                self.resolver = nil
                self.startWebSocketConnection(to: resolved ?? endpoint, path: path, name: name)
            case .failed, .cancelled:
                resolver.cancel()
                self.resolver = nil
            default:
                break
            }
        }
        resolver.start(queue: .main)
    }

    private func startWebSocketConnection(to endpoint: NWEndpoint, path: String, name: String) {
        let target: NWEndpoint
        if case .hostPort(let host, let port) = endpoint, let url = Self.webSocketURL(host: host, port: port, path: path) {
            target = .url(url)
        } else {
            target = endpoint
        }

        let webSocketOptions = NWProtocolWebSocket.Options()
        webSocketOptions.autoReplyPing = true

        let parameters = NWParameters.tcp
        parameters.defaultProtocolStack.applicationProtocols.insert(webSocketOptions, at: 0)

        let connection = NWConnection(to: target, using: parameters)
        self.connection = connection
        connection.stateUpdateHandler = { [weak self] connectionState in
            guard let self else { return }
            switch connectionState {
            case .ready:
                self.state = .connected(name: name)
                self.receive(on: connection, name: name)
            case .failed, .cancelled:
                if case .disconnected = self.state {} else {
                    self.state = .failed(name: name)
                }
                self.failPendingReply(with: EndpointConnectionError.connectionFailed)
            default:
                break
            }
        }
        connection.start(queue: .main)
    }

    private static func webSocketURL(host: NWEndpoint.Host, port: NWEndpoint.Port, path: String) -> URL? {
        let hostString: String
        switch host {
        case .ipv4(let address):
            hostString = Self.stripInterfaceSuffix(address.debugDescription)
        case .ipv6(let address):
            hostString = "[\(Self.stripInterfaceSuffix(address.debugDescription))]"
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

    public func disconnect() {
        resolver?.cancel()
        resolver = nil
        connection?.cancel()
        connection = nil
        state = .disconnected
        failPendingReply(with: EndpointConnectionError.notConnected)
    }

    public func send(_ message: String) async throws -> String {
        guard let connection, case .connected = state else {
            throw EndpointConnectionError.notConnected
        }
        guard pendingReply == nil else {
            throw EndpointConnectionError.sendInProgress
        }

        return try await withCheckedThrowingContinuation { continuation in
            pendingReply = continuation

            let metadata = NWProtocolWebSocket.Metadata(opcode: .text)
            let context = NWConnection.ContentContext(identifier: "text", metadata: [metadata])

            connection.send(
                content: Data(message.utf8),
                contentContext: context,
                isComplete: true,
                completion: .contentProcessed { [weak self] error in
                    if let error {
                        self?.resumePendingReply(with: .failure(error))
                    }
                }
            )
        }
    }

    public func sendOneWay(_ message: String) async throws {
        guard let connection, case .connected = state else {
            throw EndpointConnectionError.notConnected
        }

        try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
            let metadata = NWProtocolWebSocket.Metadata(opcode: .text)
            let context = NWConnection.ContentContext(identifier: "text", metadata: [metadata])

            connection.send(
                content: Data(message.utf8),
                contentContext: context,
                isComplete: true,
                completion: .contentProcessed { error in
                    if let error {
                        continuation.resume(throwing: error)
                    } else {
                        continuation.resume()
                    }
                }
            )
        }
    }

    private func receive(on connection: NWConnection, name: String) {
        connection.receiveMessage { [weak self] content, contentContext, _, error in
            guard let self else { return }
            if let error {
                if case .connected = self.state {
                    self.state = .failed(name: name)
                }
                self.resumePendingReply(with: .failure(error))
                return
            }

            if let content,
               let metadata = contentContext?.protocolMetadata(definition: NWProtocolWebSocket.definition) as? NWProtocolWebSocket.Metadata,
               metadata.opcode == .text {
                if let text = String(data: content, encoding: .utf8) {
                    self.resumePendingReply(with: .success(text))
                } else {
                    self.resumePendingReply(with: .failure(EndpointConnectionError.unexpectedMessage))
                }
            }

            self.receive(on: connection, name: name)
        }
    }

    private func resumePendingReply(with result: Result<String, Error>) {
        guard let pendingReply else { return }
        self.pendingReply = nil
        pendingReply.resume(with: result)
    }

    private func failPendingReply(with error: Error) {
        resumePendingReply(with: .failure(error))
    }
}
