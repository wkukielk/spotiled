//
//  PairControllerView.swift
//  spotiled-connect
//
//  Created by wkuki on 13/09/2026.
//

import Network
import SwiftUI
import SpotiLEDKit

/// Shown the first time the user selects a controller that hasn't been paired yet. Runs the
/// `/pair` SAS handshake and requires the user to *type* the code shown in the controller's log —
/// rather than just tapping a "Matches" button — before calling `onPaired` and dismissing. Typing
/// forces the user to actually look at the log, instead of reflexively confirming two
/// similar-looking numbers without really comparing them.
struct PairControllerView: View {
    let endpoint: NWEndpoint
    let name: String
    let onPaired: () -> Void

    @State private var pairing: any ControllerPairing
    @State private var enteredCode = ""
    @State private var showMismatch = false
    @Environment(\.dismiss) private var dismiss

    init(
        endpoint: NWEndpoint,
        name: String,
        pairing: any ControllerPairing = ControllerPairingSession(),
        onPaired: @escaping () -> Void
    ) {
        self.endpoint = endpoint
        self.name = name
        self.onPaired = onPaired
        _pairing = State(initialValue: pairing)
    }

    var body: some View {
        VStack(spacing: 24) {
            switch pairing.state {
            case .idle, .connecting:
                ProgressView("Connecting to \(name)…")
            case .awaitingConfirmation(let code):
                confirmationView(expectedCode: code)
            case .paired:
                ContentUnavailableView("Paired", systemImage: "checkmark.circle")
            case .rejectedByUser:
                ContentUnavailableView("Pairing Cancelled", systemImage: "xmark.circle")
            case .failed(let message):
                ContentUnavailableView {
                    Label("Pairing Failed", systemImage: "exclamationmark.triangle")
                } description: {
                    Text(message)
                }
            }
        }
        .padding()
        .task {
            pairing.begin(endpoint: endpoint, name: name)
        }
        .onChange(of: isPaired) { _, paired in
            if paired {
                onPaired()
                dismiss()
            }
        }
    }

    private func confirmationView(expectedCode: String) -> some View {
        VStack(spacing: 16) {
            Text("Check \(name)'s log")
                .font(.headline)
            Text("Type the verification code printed there.")
                .font(.subheadline)
                .multilineTextAlignment(.center)
                .foregroundStyle(.secondary)

            TextField("123 456", text: $enteredCode)
                .textFieldStyle(.roundedBorder)
                .keyboardType(.numberPad)
                .multilineTextAlignment(.center)
                .font(.system(.title2, design: .monospaced))
                .frame(maxWidth: 200)
                .onSubmit { submit(expectedCode: expectedCode) }

            if showMismatch {
                Text("That doesn't match. Check the log and try again.")
                    .font(.footnote)
                    .foregroundStyle(.red)
            }

            HStack(spacing: 16) {
                Button("Cancel", role: .destructive) {
                    Task { await pairing.reject() }
                }
                Button("Submit") {
                    submit(expectedCode: expectedCode)
                }
                .buttonStyle(.borderedProminent)
                .disabled(enteredCode.isEmpty)
            }
        }
    }

    private func submit(expectedCode: String) {
        guard Self.digitsOnly(enteredCode) == Self.digitsOnly(expectedCode) else {
            showMismatch = true
            return
        }
        showMismatch = false
        Task { await pairing.confirm() }
    }

    private static func digitsOnly(_ text: String) -> String {
        text.filter(\.isNumber)
    }

    private var isPaired: Bool {
        if case .paired = pairing.state { return true }
        return false
    }
}

#Preview("Awaiting confirmation") {
    PairControllerView(
        endpoint: .hostPort(host: "192.168.1.50", port: 8080),
        name: "Living Room",
        pairing: PreviewPairing(state: .awaitingConfirmation(code: "482 917")),
        onPaired: {}
    )
}

#Preview("Failed") {
    PairControllerView(
        endpoint: .hostPort(host: "192.168.1.50", port: 8080),
        name: "Living Room",
        pairing: PreviewPairing(state: .failed("Could not connect to the controller.")),
        onPaired: {}
    )
}
