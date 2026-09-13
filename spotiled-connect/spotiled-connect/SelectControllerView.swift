//
//  SelectControllerView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import Network
import SwiftUI
import SpotiLEDKit

struct SelectControllerView: View {
    private let browser: DeviceBrowser
    @Environment(\.esp32Session) private var session
    @Environment(\.dismiss) private var dismiss
    @State private var pairingTarget: DiscoveredDevice?
    @State private var certificateMismatch: DiscoveredDevice?
    /// The device passed to the most recent `connect()` call — used to resolve `.certificateMismatch`
    /// instead of re-searching `browser.discoveredDevices`, since that live Bonjour list may no
    /// longer contain the controller by the time a mismatch is reported (it can take several
    /// seconds, and a controller with a changed identity may not still be announcing under the
    /// same record it was when the user tapped it).
    @State private var lastConnectedDevice: DiscoveredDevice?

    init(browser: DeviceBrowser = DeviceBrowser()) {
        self.browser = browser
    }

    var body: some View {
        DiscoveredDeviceListView(
            scanner: browser,
            title: "Select controller",
            emptyMessage: "Searching for controllers…"
        ) { controller in
            listItem(controller)
        }
        .onChange(of: session.controllerState) { _, newState in
            switch newState {
                case .connected, .failed:
                    dismiss()
                case .certificateMismatch(let name):
                    certificateMismatch = lastConnectedDevice?.name == name
                        ? lastConnectedDevice
                        : browser.discoveredDevices.first { $0.name == name }
                default:
                    break
            }
        }
        .sheet(item: $pairingTarget) { controller in
            PairControllerView(endpoint: controller.endpoint, name: controller.name) {
                lastConnectedDevice = controller
                session.connect(to: controller.endpoint, name: controller.name)
            }
        }
        .alert(
            "Controller Certificate Changed",
            isPresented: Binding(get: { certificateMismatch != nil }, set: { if !$0 { certificateMismatch = nil } }),
            presenting: certificateMismatch
        ) { controller in
            Button("Cancel", role: .cancel) {}
            Button("Forget & Re-pair", role: .destructive) {
                session.forgetPairing(name: controller.name)
                pairingTarget = controller
            }
        } message: { controller in
            Text("\(controller.name)'s certificate doesn't match what was saved. This can happen if the controller was reset or reinstalled — but it can also mean something is intercepting the connection. Only continue if you're sure why it changed.")
        }
    }

    private func listItem(_ controller: DiscoveredDevice) -> some View {
        Button {
            selectController(controller)
        } label: {
            HStack {
                Text(controller.name)
                Spacer()
                if isConnecting(to: controller) {
                    ProgressView()
                }
            }
        }
    }

    private func selectController(_ controller: DiscoveredDevice) {
        lastConnectedDevice = controller
        if session.isPaired(name: controller.name) {
            session.connect(to: controller.endpoint, name: controller.name)
        } else {
            pairingTarget = controller
        }
    }

    private func isConnecting(to controller: DiscoveredDevice) -> Bool {
        if case .connecting(let name) = session.controllerState {
            return name == controller.name
        }
        return false
    }
}

#Preview {
    NavigationStack {
        SelectControllerView()
            .environment(\.esp32Session, ControllerESP32Session())
    }
}

#Preview("Connection fails") {
    SelectControllerDismissHarness(controllers: [
        DiscoveredDevice(name: "Living Room", endpoint: .hostPort(host: "192.168.1.50", port: 8080)),
        DiscoveredDevice(name: "Bedroom", endpoint: .hostPort(host: "192.168.1.51", port: 8080))
    ])
    .environment(\.esp32Session, PreviewFailingConnection())
}

#Preview("Certificate mismatch") {
    SelectControllerDismissHarness(controllers: [
        DiscoveredDevice(name: "Living Room", endpoint: .hostPort(host: "192.168.1.50", port: 8080))
    ])
    .environment(\.esp32Session, PreviewCertificateMismatchConnection())
}
