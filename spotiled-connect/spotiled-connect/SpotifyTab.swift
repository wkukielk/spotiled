//
//  SpotifyTab.swift
//  spotiled-connect
//
//  Created by wkuki on 26/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct SpotifyTab: View {
    @Environment(SelectedESP32Store.self) private var esp32Store
    @Environment(\.esp32Session) private var session
    @State private var hostResolver = DeviceHostResolver()
    @State private var connectionError: String?

    var body: some View {
        VStack {
            Text("Spotify mode")
                .navigationTitle("Manual mode")
            if let device = esp32Store.device {
                Text("Target: \(device.name)")
                    .foregroundStyle(.secondary)
            } else {
                Text("No ESP32 selected")
                    .foregroundStyle(.secondary)
            }
            if let connectionError {
                Text(connectionError)
                    .foregroundStyle(.red)
            }
        }
        .padding(.horizontal)
        .task(id: esp32Store.device) {
            connectionError = nil
            guard let device = esp32Store.device else { return }
            guard let host = await hostResolver.resolveHost(for: device.endpoint) else {
                connectionError = "Could not resolve \(device.name)"
                return
            }
            do {
                try await session.connectESP32(hostname: host)
            } catch {
                connectionError = "Failed to connect to \(device.name): \(error)"
            }
        }
    }
}

#Preview {
    SpotifyTab()
        .environment(SelectedESP32Store())
}


