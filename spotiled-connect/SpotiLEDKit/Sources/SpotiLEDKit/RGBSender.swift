//
//  RGBSender.swift
//  SpotiLEDKit
//
//  Created by wkuki on 24/07/2026.
//

import SwiftUI
import Network
import Mockable

public struct RGBColor: Equatable {
    public let red: UInt8
    public let green: UInt8
    public let blue: UInt8

    public init(red: UInt8, green: UInt8, blue: UInt8) {
        self.red = red
        self.green = green
        self.blue = blue
    }
}

public extension RGBColor {
    init(_ color: Color) {
        let resolved = color.resolve(in: EnvironmentValues())
        self.init(
            red: UInt8(clamping: Int((resolved.red * 255).rounded())),
            green: UInt8(clamping: Int((resolved.green * 255).rounded())),
            blue: UInt8(clamping: Int((resolved.blue * 255).rounded()))
        )
    }
}

@Mockable
public protocol RGBSending {
    func send(_ color: RGBColor)
}

public final class UDPRGBSender: RGBSending {
    public static let defaultPort: NWEndpoint.Port = 4210

    private let host: NWEndpoint.Host
    private let port: NWEndpoint.Port

    public init(host: NWEndpoint.Host = "192.168.1.1", port: NWEndpoint.Port = UDPRGBSender.defaultPort) {
        self.host = host
        self.port = port
    }

    public func send(_ color: RGBColor) {
        // TODO: open an NWConnection(.udp) to host:port and write [color.red, color.green, color.blue].
    }
}
