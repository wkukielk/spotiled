//
//  ManualColorControllerTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 24/07/2026.
//

import SwiftUI
import Testing
import Mockable
@testable import SpotiLEDKit

struct ManualColorControllerTests {

    @Test func sendsRGBWhenColorChanges() {
        let sender = MockRGBSending(policy: .relaxed)
        let controller = ManualColorController(sender: sender)

        controller.color = .red

        verify(sender).send(.value(RGBColor(.red))).called(.once)
    }

    @Test func doesNotSendOnInit() {
        let sender = MockRGBSending(policy: .relaxed)
        _ = ManualColorController(sender: sender)

        verify(sender).send(.any).called(.never)
    }
}
