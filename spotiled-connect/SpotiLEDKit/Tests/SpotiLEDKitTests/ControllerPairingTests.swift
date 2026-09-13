//
//  ControllerPairingTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 13/09/2026.
//

import Foundation
import Testing
import Mockable
import Network
import CryptoKit
@testable import SpotiLEDKit

struct ControllerPairingTests {
    /// Pins the Swift implementation to the same output as the controller's
    /// `compute_sas_matches_the_swift_client_golden_vector` Rust test — if this ever fails, the
    /// two `compute_sas` implementations have drifted apart and pairing will show different codes
    /// on each end.
    @Test func computeSASMatchesTheRustGoldenVector() {
        let a = Data(repeating: 1, count: 32)
        let b = Data(repeating: 2, count: 32)
        let s = Data(repeating: 3, count: 32)
        let fp = Data(repeating: 4, count: 32)

        #expect(ControllerPairingSession.computeSAS(clientPub: a, serverPub: b, shared: s, certFingerprint: fp) == "061 920")
    }

    @Test func computeSASDependsOnEveryInput() {
        let a = Data(repeating: 1, count: 32)
        let b = Data(repeating: 2, count: 32)
        let s = Data(repeating: 3, count: 32)
        let fp = Data(repeating: 4, count: 32)
        let other = Data(repeating: 9, count: 32)

        let base = ControllerPairingSession.computeSAS(clientPub: a, serverPub: b, shared: s, certFingerprint: fp)

        #expect(ControllerPairingSession.computeSAS(clientPub: other, serverPub: b, shared: s, certFingerprint: fp) != base)
        #expect(ControllerPairingSession.computeSAS(clientPub: a, serverPub: other, shared: s, certFingerprint: fp) != base)
        #expect(ControllerPairingSession.computeSAS(clientPub: a, serverPub: b, shared: other, certFingerprint: fp) != base)
        #expect(ControllerPairingSession.computeSAS(clientPub: a, serverPub: b, shared: s, certFingerprint: other) != base)
    }
}
