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

    let onSelect: (DiscoveredDevice) -> Void

    var body: some View {
        DiscoveredDeviceListView(
            scanner: DeviceBrowser(discovery: BonjourServiceDiscovery(serviceType: "_s-led-direct._udp")),
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
