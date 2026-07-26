//
//  SelectESP32View.swift
//  spotiled-connect
//
//  Created by wkuki on 26/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct SelectESP32View: View {
    @State private var browser = ControllerBrowser(discovery: BonjourControllerDiscovery(serviceType: "_s-led-ledstrip._tcp"))
    @Environment(\.dismiss) private var dismiss

    let onSelect: (DiscoveredController) -> Void

    var body: some View {
        List(browser.discoveredControllers) { controller in
            Button {
                onSelect(controller)
                dismiss()
            } label: {
                Text(controller.name)
            }
        }
        .overlay {
            if browser.discoveredControllers.isEmpty {
                ContentUnavailableView("Searching for ESP32s…", systemImage: "wifi")
            }
        }
        .navigationTitle("Select ESP32")
        .task {
            browser.start()
        }
        .onDisappear {
            browser.stop()
        }
    }
}

#Preview {
    NavigationStack {
        SelectESP32View(onSelect: { _ in })
    }
}
