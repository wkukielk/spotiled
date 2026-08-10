//
//  ESP32Session.swift
//  SpotiLEDKit
//
//  Created by wkuki on 10/08/2026.
//

import Foundation
import Network
import Observation
import SwiftUI
import Mockable

/// Whether the *controller* currently has its own proxied websocket open to an ESP32.
/// The ESP32 accepts one client at a time, so this is tracked separately from
/// `controllerState`, which is the phone's own link to the controller.
public enum ESP32ConnectionState: Equatable, Sendable {
    case disconnected
    case connecting(host: String)
    case connected(host: String)
    case failed(host: String, message: String)
}

public enum ESP32SessionError: Error, Equatable {
    case rejected(String)
}

/// Everything a view needs to know about the controller: the phone's own link to it,
/// and the ESP32 the controller is (or isn't) proxying on the phone's behalf. Wraps a
/// `ControllerConnecting` transport and speaks the controller's proxy protocol over it
/// (`connect <hostname>` / `disconnect`, acked with `{"ack":"ok"}` or `{"error": ...}`).
@Mockable
public protocol ESP32SessionManaging: AnyObject {
    var controllerState: ControllerConnectionState { get }
    var esp32State: ESP32ConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String)
    func disconnect()
    func connectESP32(hostname: String) async throws
    func disconnectESP32() async throws
}

@Observable
public final class ControllerESP32Session: ESP32SessionManaging {
    private let connection: any ControllerConnecting
    public private(set) var esp32State: ESP32ConnectionState = .disconnected

    public var controllerState: ControllerConnectionState { connection.state }

    public init(connection: any ControllerConnecting = WebSocketControllerConnection()) {
        self.connection = connection
    }

    public func connect(to endpoint: NWEndpoint, name: String) {
        connection.connect(to: endpoint, name: name)
    }

    public func disconnect() {
        connection.disconnect()
        esp32State = .disconnected
    }

    public func connectESP32(hostname: String) async throws {
        esp32State = .connecting(host: hostname)
        do {
            let reply = try await connection.send("connect \(hostname)")
            if let message = Self.errorMessage(in: reply) {
                esp32State = .failed(host: hostname, message: message)
                throw ESP32SessionError.rejected(message)
            }
            esp32State = .connected(host: hostname)
        } catch {
            if case .connecting = esp32State {
                esp32State = .failed(host: hostname, message: "\(error)")
            }
            throw error
        }
    }

    public func disconnectESP32() async throws {
        let reply = try await connection.send("disconnect")
        if let message = Self.errorMessage(in: reply) {
            throw ESP32SessionError.rejected(message)
        }
        esp32State = .disconnected
    }

    private struct ControllerReply: Decodable {
        let ack: String?
        let error: String?
    }

    private static func errorMessage(in reply: String) -> String? {
        guard let data = reply.data(using: .utf8),
              let decoded = try? JSONDecoder().decode(ControllerReply.self, from: data) else {
            return nil
        }
        return decoded.error
    }
}

private struct ESP32SessionKey: EnvironmentKey {
    static let defaultValue: any ESP32SessionManaging = ControllerESP32Session()
}

public extension EnvironmentValues {
    var esp32Session: any ESP32SessionManaging {
        get { self[ESP32SessionKey.self] }
        set { self[ESP32SessionKey.self] = newValue }
    }
}
