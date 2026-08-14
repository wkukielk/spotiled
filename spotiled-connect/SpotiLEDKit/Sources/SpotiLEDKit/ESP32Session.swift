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

public enum ESP32ConnectionState: Equatable, Sendable {
    case disconnected
    case connecting(host: String)
    case connected(host: String)
    case failed(host: String, message: String)
}

public enum ESP32SessionError: Error, Equatable {
    case rejected(String)
}

@Mockable
public protocol ControllerSessionManaging: AnyObject {
    var controllerState: EndpointConnectionState { get }
    var esp32State: ESP32ConnectionState { get }
    func connect(to endpoint: NWEndpoint, name: String)
    func disconnect()
    func connectESP32(hostname: String) async throws
    func disconnectESP32() async throws
}

@Observable
public final class ControllerESP32Session: ControllerSessionManaging {
    private let connection: any EndpointConnecting
    public private(set) var esp32State: ESP32ConnectionState = .disconnected

    public var controllerState: EndpointConnectionState { connection.state }

    public init(connection: any EndpointConnecting = WebSocketEndpointConnection()) {
        self.connection = connection
    }

    public func connect(to endpoint: NWEndpoint, name: String) {
        connection.connect(to: endpoint, name: name, path: "/")
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
    static let defaultValue: any ControllerSessionManaging = ControllerESP32Session()
}

public extension EnvironmentValues {
    var esp32Session: any ControllerSessionManaging {
        get { self[ESP32SessionKey.self] }
        set { self[ESP32SessionKey.self] = newValue }
    }
}
