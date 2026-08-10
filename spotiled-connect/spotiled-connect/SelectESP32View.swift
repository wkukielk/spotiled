//
//  SelectESP32View.swift
//  spotiled-connect
//
//  Created by wkuki on 26/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct SelectESP32View: View {
    @Environment(\.dismiss) private var dismiss

    let onSelect: (DiscoveredController) -> Void

    var body: some View {
        DiscoveredControllerListView(
            browser: ControllerBrowser(discovery: BonjourControllerDiscovery(serviceType: "_s-led-auto._tcp")),
            title: "Select ESP32",
            emptyMessage: "Searching for ESP32s…"
        ) { controller in
            Button {
                onSelect(controller)
                dismiss()
            } label: {
                Text(controller.name)
            }
        }
    }
}

#Preview {
    NavigationStack {
        SelectESP32View(onSelect: { _ in })
    }
}
