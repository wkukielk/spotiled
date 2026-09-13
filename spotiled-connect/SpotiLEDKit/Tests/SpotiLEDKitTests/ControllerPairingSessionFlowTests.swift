//
//  ControllerPairingSessionFlowTests.swift
//  SpotiLEDKitTests
//
//  Created by wkuki on 13/09/2026.
//

import CryptoKit
import Foundation
import Mockable
import Network
import Testing
@testable import SpotiLEDKit

struct ControllerPairingSessionFlowTests {
    /// A fake "server": generates its own ephemeral key, replies to a commit with a reveal, and
    /// remembers what the client revealed so a test can compute the SAS it expects the session to
    /// derive.
    private final class FakeServer {
        let privateKey = Curve25519.KeyAgreement.PrivateKey()
        var revealedClientPub: Data?

        func reveal() -> String {
            #"{"type":"reveal","pub":"\#(privateKey.publicKey.rawRepresentation.base64EncodedString())"}"#
        }
    }

    private static func makeConnection(server: FakeServer, fingerprint: Data) -> MockEndpointConnecting {
        let connection = MockEndpointConnecting(policy: .relaxed)
        given(connection).state.willReturn(.connected(name: "Controller"))
        given(connection).connect(to: .any, name: .any, path: .value("/pair"), tls: .any).willProduce { _, _, _, tls in
            if case .pairing(let onFingerprint) = tls {
                onFingerprint(fingerprint)
            }
        }
        given(connection).send(.any).willProduce { message in
            guard let data = message.data(using: .utf8),
                  let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] else {
                return "{}"
            }
            switch json["type"] as? String {
            case "commit":
                return server.reveal()
            case "confirm":
                return #"{"type":"paired"}"#
            case "reject":
                return #"{"type":"rejected"}"#
            default:
                return "{}"
            }
        }
        given(connection).sendOneWay(.any).willProduce { message in
            guard let data = message.data(using: .utf8),
                  let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
                  let pubBase64 = json["pub"] as? String else {
                return
            }
            server.revealedClientPub = Data(base64Encoded: pubBase64)
        }
        return connection
    }

    private static func makeTrustStore(function: String) -> ControllerTrustStore {
        let defaults = UserDefaults(suiteName: function)!
        defaults.removePersistentDomain(forName: function)
        return ControllerTrustStore(defaults: defaults)
    }

    private func awaitState(_ pairing: ControllerPairingSession, timeout: TimeInterval = 1) async -> PairingState {
        let deadline = Date().addingTimeInterval(timeout)
        while Date() < deadline {
            if case .connecting = pairing.state {
                try? await Task.sleep(nanoseconds: 5_000_000)
                continue
            }
            return pairing.state
        }
        return pairing.state
    }

    @Test func beginDerivesTheSameSASBothSidesWouldCompute() async throws {
        let server = FakeServer()
        let fingerprint = Data(repeating: 0x42, count: 32)
        let connection = Self.makeConnection(server: server, fingerprint: fingerprint)
        let trustStore = Self.makeTrustStore(function: #function)
        let pairing = ControllerPairingSession(connection: connection, trustStore: trustStore)

        pairing.begin(endpoint: .hostPort(host: "192.168.1.10", port: 1), name: "Controller")

        let state = await awaitState(pairing)
        guard case .awaitingConfirmation(let code) = state else {
            Issue.record("expected .awaitingConfirmation, got \(state)")
            return
        }

        // Recompute the SAS the way the "server" side would, using what the client actually
        // revealed, and check it matches what the session derived.
        guard let clientPub = server.revealedClientPub else {
            Issue.record("server never received the client's reveal")
            return
        }
        let serverPrivate = server.privateKey
        let clientPublicKey = try Curve25519.KeyAgreement.PublicKey(rawRepresentation: clientPub)
        let shared = try serverPrivate.sharedSecretFromKeyAgreement(with: clientPublicKey)
        let sharedBytes = shared.withUnsafeBytes { Data($0) }
        let expected = ControllerPairingSession.computeSAS(
            clientPub: clientPub,
            serverPub: serverPrivate.publicKey.rawRepresentation,
            shared: sharedBytes,
            certFingerprint: fingerprint
        )

        #expect(code == expected)
    }

    @Test func confirmPinsTheCertificateFingerprintAndTransitionsToPaired() async throws {
        let server = FakeServer()
        let fingerprint = Data(repeating: 0x7, count: 32)
        let connection = Self.makeConnection(server: server, fingerprint: fingerprint)
        let trustStore = Self.makeTrustStore(function: #function)
        let pairing = ControllerPairingSession(connection: connection, trustStore: trustStore)

        pairing.begin(endpoint: .hostPort(host: "192.168.1.10", port: 1), name: "Controller")
        _ = await awaitState(pairing)

        await pairing.confirm()

        #expect(pairing.state == .paired)
        #expect(trustStore.pinnedFingerprint(for: "Controller") == fingerprint)
    }

    @Test func rejectDoesNotPinAndTransitionsToRejectedByUser() async throws {
        let server = FakeServer()
        let fingerprint = Data(repeating: 0x9, count: 32)
        let connection = Self.makeConnection(server: server, fingerprint: fingerprint)
        let trustStore = Self.makeTrustStore(function: #function)
        let pairing = ControllerPairingSession(connection: connection, trustStore: trustStore)

        pairing.begin(endpoint: .hostPort(host: "192.168.1.10", port: 1), name: "Controller")
        _ = await awaitState(pairing)

        await pairing.reject()

        #expect(pairing.state == .rejectedByUser)
        #expect(trustStore.pinnedFingerprint(for: "Controller") == nil)
    }
}
