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
    private let browser: ControllerBrowser
    @Environment(\.controllerConnection) private var connection
    @Environment(\.dismiss) private var dismiss

    init(browser: ControllerBrowser = ControllerBrowser()) {
        self.browser = browser
    }

    var body: some View {
        DiscoveredControllerListView(
            browser: browser,
            title: "Select controller",
            emptyMessage: "Searching for controllers…"
        ) { controller in
            listItem(controller)
        }
        .onChange(of: connection.state) { _, newState in
            switch newState {
                case .connected, .failed:
                    dismiss()
                default:
                    break
            }
        }
    }

    private func listItem(_ controller: DiscoveredController) -> some View {
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

    private func selectController(_ controller: DiscoveredController) {
        connection.connect(to: controller.endpoint, name: controller.name)
    }

    private func isConnecting(to controller: DiscoveredController) -> Bool {
        if case .connecting(let name) = connection.state {
            return name == controller.name
        }
        return false
    }
}

#Preview {
    NavigationStack {
        SelectControllerView()
            .environment(\.controllerConnection, WebSocketControllerConnection())
    }
}

#Preview("Connection fails") {
    SelectControllerDismissHarness(controllers: [
        DiscoveredController(name: "Living Room", endpoint: .hostPort(host: "192.168.1.50", port: 8080)),
        DiscoveredController(name: "Bedroom", endpoint: .hostPort(host: "192.168.1.51", port: 8080))
    ])
    .environment(\.controllerConnection, PreviewFailingConnection())
}
