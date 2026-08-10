//
//  ControllerConnection.swift
//  SpotiLEDKit
//
//  Created by wkuki on 25/07/2026.
//

import Foundation
import Network
import Observation
import Mockable

public enum ControllerConnectionState: Equatable, Sendable {
    case disconnected
    case connecting(name: String)
    case connected(name: String)
    case failed(name: String)
}

public enum ControllerConnectionError: Error, Equatable {
    case notConnected
    case sendInProgress
    case connectionFailed
    case unexpectedMessage
}

@Mockable
public protocol ControllerConnecting: AnyObject {
    var state: ControllerConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String)
    func disconnect()
    func send(_ message: String) async throws -> String
}

@Observable
public final class WebSocketControllerConnection: ControllerConnecting, @unchecked Sendable {
    public private(set) var state: ControllerConnectionState = .disconnected

    private var connection: NWConnection?
    private var pendingReply: CheckedContinuation<String, Error>?

    public init() {}

    public func connect(to endpoint: NWEndpoint, name: String) {
        disconnect()
        state = .connecting(name: name)

        let webSocketOptions = NWProtocolWebSocket.Options()
        webSocketOptions.autoReplyPing = true

        let parameters = NWParameters.tcp
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
                self.failPendingReply(with: ControllerConnectionError.connectionFailed)
            default:
                break
            }
        }
        connection.start(queue: .main)
    }

    public func disconnect() {
        connection?.cancel()
        connection = nil
        state = .disconnected
        failPendingReply(with: ControllerConnectionError.notConnected)
    }

    public func send(_ message: String) async throws -> String {
        guard let connection, case .connected = state else {
            throw ControllerConnectionError.notConnected
        }
        guard pendingReply == nil else {
            throw ControllerConnectionError.sendInProgress
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
                    self.resumePendingReply(with: .failure(ControllerConnectionError.unexpectedMessage))
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
