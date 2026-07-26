//
//  ControllerConnection.swift
//  SpotiLEDKit
//
//  Created by wkuki on 25/07/2026.
//

import Network
import Observation
import SwiftUI
import Mockable

public enum ControllerConnectionState: Equatable, Sendable {
    case disconnected
    case connecting(name: String)
    case connected(name: String)
    case failed(name: String)
}

@Mockable
public protocol ControllerConnecting: AnyObject {
    var state: ControllerConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String)
    func disconnect()
}

@Observable
public final class WebSocketControllerConnection: ControllerConnecting {
    public private(set) var state: ControllerConnectionState = .disconnected

    private var connection: NWConnection?

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
    }

    private func receive(on connection: NWConnection, name: String) {
        connection.receiveMessage { [weak self] _, _, _, error in
            guard let self else { return }
            if error != nil {
                if case .connected = self.state {
                    self.state = .failed(name: name)
                }
                return
            }
            self.receive(on: connection, name: name)
        }
    }
}

private struct ControllerConnectionKey: EnvironmentKey {
    static let defaultValue: any ControllerConnecting = WebSocketControllerConnection()
}

public extension EnvironmentValues {
    var controllerConnection: any ControllerConnecting {
        get { self[ControllerConnectionKey.self] }
        set { self[ControllerConnectionKey.self] = newValue }
    }
}
