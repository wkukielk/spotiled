//
//  DirectESP32Connection.swift
//  SpotiLEDKit
//
//  Created by wkuki on 13/08/2026.
//

import Foundation
import Network
import Observation
import Mockable

@Mockable
public protocol DirectESP32Connecting: AnyObject {
    var state: EndpointConnectionState { get }
    func connect(to device: DiscoveredDevice)
    func disconnect()
    func pulsate(color: RGBColor, durationMs: Int32) async throws
}

@Observable
public final class DirectESP32Connection: DirectESP32Connecting {
    private let connection: any EndpointConnecting

    public var state: EndpointConnectionState { connection.state }

    public init(connection: any EndpointConnecting = WebSocketEndpointConnection()) {
        self.connection = connection
    }

    public func connect(to device: DiscoveredDevice) {
        connection.connect(to: device.endpoint, name: device.name)
    }

    public func disconnect() {
        connection.disconnect()
    }

    public func pulsate(color: RGBColor, durationMs: Int32) async throws {
        let command = PulsateCommand(r: color.red, g: color.green, b: color.blue, durationMs: durationMs)
        let data = try JSONEncoder().encode(command)
        _ = try await connection.send(String(decoding: data, as: UTF8.self))
    }

    private struct PulsateCommand: Encodable {
        let cmd = "pulsate"
        let r: UInt8
        let g: UInt8
        let b: UInt8
        let durationMs: Int32

        enum CodingKeys: String, CodingKey {
            case cmd, r, g, b
            case durationMs = "duration_ms"
        }
    }
}
