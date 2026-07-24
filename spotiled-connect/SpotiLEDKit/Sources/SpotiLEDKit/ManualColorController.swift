//
//  ManualColorController.swift
//  SpotiLEDKit
//
//  Created by wkuki on 24/07/2026.
//

import SwiftUI

@Observable
public final class ManualColorController {
    public var color: Color = Color(.mint) {
        didSet {
            sender.send(RGBColor(color))
        }
    }
    private let sender: RGBSending

    public init(sender: RGBSending = UDPRGBSender()) {
        self.sender = sender
    }
}
