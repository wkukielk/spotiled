//
//  EndpointConnection.swift
//  SpotiLEDKit
//
//  Created by wkuki on 25/07/2026.
//

import CryptoKit
import Foundation
import Network
import Observation
import Mockable
import Security

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

/// How a connection should handle TLS trust. `.insecure` is today's plaintext behavior (used for
/// the direct ESP32 connection). `.pairing`/`.pinned` are for the controller connection: `.pairing`
/// is TOFU — any certificate is accepted, but its SHA-256 fingerprint is reported out so the
/// pairing flow can show it as a SAS and, once confirmed, pin it; `.pinned` only completes the
/// handshake if the presented certificate's fingerprint matches exactly.
public enum ControllerTLSPolicy {
    case insecure
    case pairing(onFingerprint: (Data) -> Void)
    case pinned(Data)
}

@Mockable
public protocol EndpointConnecting: AnyObject {
    var state: EndpointConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String, path: String, tls: ControllerTLSPolicy)
    func disconnect()
    func send(_ message: String) async throws -> String
    func sendOneWay(_ message: String) async throws
}

public extension EndpointConnecting {
    /// Convenience for callers that don't need TLS (e.g. the direct ESP32 connection).
    func connect(to endpoint: NWEndpoint, name: String, path: String) {
        connect(to: endpoint, name: name, path: path, tls: .insecure)
    }
}

@Observable
public final class WebSocketEndpointConnection: EndpointConnecting, @unchecked Sendable {
    private static let connectTimeout: TimeInterval = 8

    public private(set) var state: EndpointConnectionState = .disconnected

    private var connection: NWConnection?
    private var pendingReply: CheckedContinuation<String, Error>?

    private let resolver = EndpointResolver()

    public init() {}

    public func connect(to endpoint: NWEndpoint, name: String, path: String, tls: ControllerTLSPolicy) {
        disconnect()
        state = .connecting(name: name)

        let secure = Self.isSecure(tls)
        resolver.resolve(endpoint, path: path, secure: secure) { [weak self] resolvedEndpoint in
            guard let self else { return }
            guard let resolvedEndpoint else {
                self.state = .failed(name: name)
                return
            }
            self.startWebSocketConnection(to: resolvedEndpoint, name: name, tls: tls)
        }
    }

    private static func isSecure(_ tls: ControllerTLSPolicy) -> Bool {
        if case .insecure = tls { return false }
        return true
    }

    private func startWebSocketConnection(to endpoint: NWEndpoint, name: String, tls: ControllerTLSPolicy) {
        let webSocketOptions = NWProtocolWebSocket.Options()
        webSocketOptions.autoReplyPing = true

        let parameters: NWParameters
        switch tls {
        case .insecure:
            parameters = .tcp
        case .pairing, .pinned:
            let tlsOptions = NWProtocolTLS.Options()
            Self.configureVerifyBlock(tlsOptions, tls: tls)
            parameters = NWParameters(tls: tlsOptions, tcp: .init())
        }
        parameters.defaultProtocolStack.applicationProtocols.insert(webSocketOptions, at: 0)

        let connection = NWConnection(to: endpoint, using: parameters)
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
        scheduleConnectTimeout(for: connection, name: name)
    }

    /// Installs a TLS verify block that never consults the system trust store — trust here is
    /// established either out-of-band (`.pairing`, confirmed by SAS) or by exact pin match
    /// (`.pinned`), never by a certificate authority, since the controller's certificate is
    /// self-signed.
    private static func configureVerifyBlock(_ tlsOptions: NWProtocolTLS.Options, tls: ControllerTLSPolicy) {
        let secOptions = tlsOptions.securityProtocolOptions
        sec_protocol_options_set_verify_block(secOptions, { _, secTrustRef, complete in
            let trust = sec_trust_copy_ref(secTrustRef).takeRetainedValue()
            guard let chain = SecTrustCopyCertificateChain(trust) as? [SecCertificate],
                  let leaf = chain.first else {
                complete(false)
                return
            }
            let der = SecCertificateCopyData(leaf) as Data
            let fingerprint = Data(SHA256.hash(data: der))

            switch tls {
            case .insecure:
                complete(true)
            case .pairing(let onFingerprint):
                onFingerprint(fingerprint)
                complete(true)
            case .pinned(let pinned):
                complete(fingerprint == pinned)
            }
        }, DispatchQueue.main)
    }

    /// A dropped SYN with no response never reaches `.failed` on its own — `NWConnection` has no
    /// built-in connect timeout, so without this a silently-lost handshake leaves `state` stuck
    /// at `.connecting` (and the UI spinning) indefinitely.
    private func scheduleConnectTimeout(for connection: NWConnection, name: String) {
        DispatchQueue.main.asyncAfter(deadline: .now() + Self.connectTimeout) { [weak self, weak connection] in
            guard let self, let connection, self.connection === connection else { return }
            guard case .connecting = self.state else { return }
            connection.cancel()
            self.state = .failed(name: name)
            self.failPendingReply(with: EndpointConnectionError.connectionFailed)
        }
    }

    public func disconnect() {
        resolver.cancel()
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
