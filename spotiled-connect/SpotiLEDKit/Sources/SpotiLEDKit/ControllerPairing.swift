//
//  ControllerPairing.swift
//  SpotiLEDKit
//
//  Created by wkuki on 13/09/2026.
//

import CryptoKit
import Foundation
import Network
import Observation
import Mockable

public enum PairingState: Equatable, Sendable {
    case idle
    case connecting
    /// `code` is the same 6-digit, space-grouped format the controller logs
    /// (`pairing::compute_sas` on the Rust side) — the user compares the two by eye.
    case awaitingConfirmation(code: String)
    case paired
    case rejectedByUser
    case failed(String)
}

@Mockable
public protocol ControllerPairing: AnyObject {
    var state: PairingState { get }
    func begin(endpoint: NWEndpoint, name: String)
    func confirm() async
    func reject() async
}

/// Drives the client side of the controller's `/pair` handshake: connects with TOFU trust
/// (`.pairing`), performs the client-commits/server-reveals ephemeral X25519 exchange described
/// in the controller's `pairing.rs`, derives the SAS for the user to compare against the
/// controller's log, and — once confirmed — pins the certificate fingerprint it observed so
/// future connections can use `.pinned` instead of running this flow again.
@Observable
public final class ControllerPairingSession: ControllerPairing {
    private let connection: any EndpointConnecting
    private let trustStore: ControllerTrustStore

    public private(set) var state: PairingState = .idle

    private var name = ""
    private var ownCertFingerprint: Data?

    public init(
        connection: any EndpointConnecting = WebSocketEndpointConnection(),
        trustStore: ControllerTrustStore = ControllerTrustStore()
    ) {
        self.connection = connection
        self.trustStore = trustStore
    }

    public func begin(endpoint: NWEndpoint, name: String) {
        self.name = name
        self.ownCertFingerprint = nil
        state = .connecting

        connection.connect(
            to: endpoint,
            name: name,
            path: "/pair",
            tls: .pairing(onFingerprint: { [weak self] fingerprint in
                self?.ownCertFingerprint = fingerprint
            })
        )

        Task { [weak self] in
            await self?.runHandshake()
        }
    }

    private func runHandshake() async {
        guard await waitUntilConnected() else {
            state = .failed("Could not connect to the controller.")
            return
        }

        let privateKey = Curve25519.KeyAgreement.PrivateKey()
        let clientPub = privateKey.publicKey.rawRepresentation

        do {
            // The server only ever sees this hash before generating its own key — see the
            // "commit-then-reveal" note in pairing.rs for why that ordering matters.
            let commitHash = Data(SHA256.hash(data: clientPub))
            let replyText = try await connection.send(PairingWireMessage(type: "commit", hash: commitHash.base64EncodedString()).encoded())

            guard let serverPubBase64 = PairingWireMessage.decode(replyText)?.pub,
                  let serverPub = Data(base64Encoded: serverPubBase64) else {
                state = .failed("Unexpected reply from the controller.")
                return
            }

            try await connection.sendOneWay(PairingWireMessage(type: "reveal", pub: clientPub.base64EncodedString()).encoded())

            guard let ownCertFingerprint else {
                state = .failed("Missing certificate fingerprint from the TLS handshake.")
                return
            }

            let serverPublicKey = try Curve25519.KeyAgreement.PublicKey(rawRepresentation: serverPub)
            let shared = try privateKey.sharedSecretFromKeyAgreement(with: serverPublicKey)
            let sharedBytes = shared.withUnsafeBytes { Data($0) }
            let sas = Self.computeSAS(clientPub: clientPub, serverPub: serverPub, shared: sharedBytes, certFingerprint: ownCertFingerprint)
            state = .awaitingConfirmation(code: sas)
        } catch {
            state = .failed("\(error)")
        }
    }

    public func confirm() async {
        guard case .awaitingConfirmation = state, let ownCertFingerprint else { return }
        do {
            let replyText = try await connection.send(PairingWireMessage(type: "confirm").encoded())
            guard PairingWireMessage.decode(replyText)?.type == "paired" else {
                state = .failed("The controller rejected pairing.")
                return
            }
            trustStore.pin(ownCertFingerprint, for: name)
            state = .paired
        } catch {
            state = .failed("\(error)")
        }
        connection.disconnect()
    }

    public func reject() async {
        guard case .awaitingConfirmation = state else { return }
        _ = try? await connection.send(PairingWireMessage(type: "reject").encoded())
        connection.disconnect()
        state = .rejectedByUser
    }

    private func waitUntilConnected() async -> Bool {
        while true {
            switch connection.state {
            case .connected:
                return true
            case .failed:
                return false
            default:
                try? await Task.sleep(nanoseconds: 20_000_000)
            }
        }
    }

    /// Must byte-for-byte match `pairing::compute_sas` in the controller's `pairing.rs` — see
    /// `compute_sas_matches_the_swift_client_golden_vector` there and
    /// `computeSASMatchesTheRustGoldenVector` in `ControllerPairingTests` for the shared fixture
    /// that pins both implementations to the same output.
    static func computeSAS(clientPub: Data, serverPub: Data, shared: Data, certFingerprint: Data) -> String {
        var hasher = SHA256()
        hasher.update(data: clientPub)
        hasher.update(data: serverPub)
        hasher.update(data: shared)
        hasher.update(data: certFingerprint)
        let digest = Data(hasher.finalize())

        let code = (UInt32(digest[0]) << 24 | UInt32(digest[1]) << 16 | UInt32(digest[2]) << 8 | UInt32(digest[3])) % 1_000_000
        return String(format: "%03d %03d", code / 1000, code % 1000)
    }
}

private struct PairingWireMessage: Codable {
    let type: String
    var hash: String?
    var pub: String?

    init(type: String, hash: String? = nil, pub: String? = nil) {
        self.type = type
        self.hash = hash
        self.pub = pub
    }

    func encoded() -> String {
        guard let data = try? JSONEncoder().encode(self), let text = String(data: data, encoding: .utf8) else {
            return "{\"type\":\"\(type)\"}"
        }
        return text
    }

    static func decode(_ text: String) -> PairingWireMessage? {
        guard let data = text.data(using: .utf8) else { return nil }
        return try? JSONDecoder().decode(PairingWireMessage.self, from: data)
    }
}
