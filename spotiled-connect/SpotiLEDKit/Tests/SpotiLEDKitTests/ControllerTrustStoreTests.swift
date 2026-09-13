//
//  ControllerTrustStoreTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 13/09/2026.
//

import Foundation
import Testing
@testable import SpotiLEDKit

struct ControllerTrustStoreTests {
    private static func makeStore(function: String = #function) -> ControllerTrustStore {
        let defaults = UserDefaults(suiteName: function)!
        defaults.removePersistentDomain(forName: function)
        return ControllerTrustStore(defaults: defaults)
    }

    @Test func returnsNilForAnUnpinnedController() {
        let store = Self.makeStore()
        #expect(store.pinnedFingerprint(for: "Living Room") == nil)
    }

    @Test func returnsWhatWasPinned() {
        let store = Self.makeStore()
        let fingerprint = Data([1, 2, 3, 4])

        store.pin(fingerprint, for: "Living Room")

        #expect(store.pinnedFingerprint(for: "Living Room") == fingerprint)
    }

    @Test func pinningIsScopedByControllerName() {
        let store = Self.makeStore()
        store.pin(Data([1]), for: "Living Room")

        #expect(store.pinnedFingerprint(for: "Bedroom") == nil)
    }

    @Test func forgetRemovesThePin() {
        let store = Self.makeStore()
        store.pin(Data([1, 2, 3]), for: "Living Room")

        store.forget("Living Room")

        #expect(store.pinnedFingerprint(for: "Living Room") == nil)
    }
}
