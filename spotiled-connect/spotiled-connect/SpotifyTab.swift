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
    @Environment(\.openURL) private var openURL
    @State private var hostResolver = DeviceHostResolver()
    @State private var connectionError: String?
    @State private var spotifyAuthorizer: any SpotifyAuthorizing = SpotifyPKCEAuthorizer()
    @State private var isAuthorizingSpotify = false
    @State private var isSpotifyAuthorized = false
    @State private var spotifyError: String?

    var body: some View {
        VStack(spacing: 16) {
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

            Button(isSpotifyAuthorized ? "Reconnect Spotify" : "Connect Spotify") {
                Task { await authorizeSpotify() }
            }
            .disabled(isAuthorizingSpotify)

            if isSpotifyAuthorized {
                Label("Spotify connected", systemImage: "checkmark.circle")
                    .foregroundStyle(.green)
            }
            if let spotifyError {
                Text(spotifyError)
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

    private func authorizeSpotify() async {
        spotifyError = nil
        isAuthorizingSpotify = true
        defer { isAuthorizingSpotify = false }
        do {
            let openURL = self.openURL
            let result = try await spotifyAuthorizer.requestAuthorizationCode { url in
                Task { @MainActor in
                    openURL(url)
                }
            }
            try await session.authorizeSpotify(code: result.code, codeVerifier: result.codeVerifier)
            isSpotifyAuthorized = true
        } catch {
            isSpotifyAuthorized = false
            spotifyError = "Failed to connect Spotify: \(error)"
        }
    }
}

#Preview {
    SpotifyTab()
        .environment(SelectedESP32Store())
}


