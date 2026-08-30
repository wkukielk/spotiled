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
    @Environment(SelectedESP32Store.self) private var esp32Store

    @State private var hostResolver = DeviceHostResolver()
    @State private var pingingDevice: DiscoveredDevice?
    @State private var pingError: String?

    var body: some View {
        DiscoveredDeviceListView(
            scanner: DeviceBrowser(discovery: BonjourServiceDiscovery(serviceType: "_s-led-direct._udp")),
            title: "Select ESP32",
            emptyMessage: "Searching for ESP32s…"
        ) { device in
            Button {
                select(device)
            } label: {
                HStack {
                    Text(device.name)
                    if pingingDevice == device {
                        Spacer()
                        ProgressView()
                    }
                }
            }
            .disabled(pingingDevice != nil)
        }
        .alert(
            "Couldn't reach device",
            isPresented: Binding(get: { pingError != nil }, set: { if !$0 { pingError = nil } })
        ) {
            Button("OK", role: .cancel) {}
        } message: {
            Text(pingError ?? "")
        }
    }

    private func select(_ device: DiscoveredDevice) {
        pingingDevice = device
        Task {
            defer { pingingDevice = nil }
            guard await hostResolver.resolveHost(for: device.endpoint) != nil else {
                pingError = "\(device.name) didn't respond. Make sure it's powered on and connected to Wi-Fi."
                return
            }
            esp32Store.device = device
            dismiss()
        }
    }
}

#Preview {
    NavigationStack {
        SelectESP32View()
            .environment(SelectedESP32Store())
    }
}
