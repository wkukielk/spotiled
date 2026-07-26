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

    var body: some View {
        VStack() {
            Text("Spotify mode")
                .navigationTitle("Manual mode")
            if let device = esp32Store.device {
                Text("Target: \(device.name)")
                    .foregroundStyle(.secondary)
            } else {
                Text("No ESP32 selected")
                    .foregroundStyle(.secondary)
            }
        }
        .padding(.horizontal)
    }
}

#Preview {
    SpotifyTab()
        .environment(SelectedESP32Store())
}


