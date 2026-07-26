//
//  SelectControllerView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct SelectControllerView: View {
    @State private var browser = ControllerBrowser()
    @Environment(\.controllerConnection) private var connection
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        List(browser.discoveredControllers) { controller in
            Button {
                selectController(controller)
            } label: {
                Text(controller.name)
            }
        }
        .overlay {
            if browser.discoveredControllers.isEmpty {
                ContentUnavailableView("Searching for controllers…", systemImage: "wifi")
            }
        }
        .navigationTitle("Select controller")
        .task {
            browser.start()
        }
        .onDisappear {
            browser.stop()
        }
        .onChange(of: connection.state) { _, newState in
            if case .connected = newState {
                dismiss()
            }
        }
    }

    private func selectController(_ controller: DiscoveredController) {
        connection.connect(to: controller.endpoint, name: controller.name)
    }
}

#Preview {
    NavigationStack {
        SelectControllerView()
            .environment(\.controllerConnection, WebSocketControllerConnection())
    }
}
