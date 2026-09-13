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
                default:
                    break
            }
        }
        .sheet(item: $pairingTarget) { controller in
            PairControllerView(endpoint: controller.endpoint, name: controller.name) {
                session.connect(to: controller.endpoint, name: controller.name)
            }
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
