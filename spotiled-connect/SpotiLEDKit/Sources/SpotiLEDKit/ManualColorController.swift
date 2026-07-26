//
//  ManualColorController.swift
//  SpotiLEDKit
//
//  Created by wkuki on 24/07/2026.
//

import SwiftUI
import Network

@Observable
public final class ManualColorController {
    public var color: Color = Color(.mint) {
        didSet {
            sender?.send(RGBColor(color))
        }
    }
    private var sender: RGBSending?

    public init(sender: RGBSending? = nil) {
        self.sender = sender
    }

    public func updateTarget(_ endpoint: NWEndpoint) {
        sender = UDPRGBSender(endpoint: endpoint)
    }
}
