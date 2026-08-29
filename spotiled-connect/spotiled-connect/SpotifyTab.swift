//
//  SpotifyTab.swift
//  spotiled-connect
//
//  Created by wkuki on 26/07/2026.
//

import SwiftUI
import SpotiLEDKit

/// Whether the controller has an active Spotify session, and the account's username if so.
/// A dedicated type instead of a plain `Bool` + `String?` pair, since those two would otherwise
/// carry the same information twice and could disagree.
private enum SpotifyStatus: Equatable {
    case checking
    case connected(username: String)
    case notConnected

    var isConnected: Bool {
        if case .connected = self { return true }
        return false
    }
}

struct SpotifyTab: View {
    @Environment(SelectedESP32Store.self) private var esp32Store
    @Environment(\.esp32Session) private var session
    @Environment(\.openURL) private var openURL
    @State private var hostResolver = DeviceHostResolver()
    @State private var connectionError: String?
    @State private var spotifyAuthorizer: any SpotifyAuthorizing = SpotifyPKCEAuthorizer()
    @State private var isAuthorizingSpotify = false
    @State private var spotifyStatus: SpotifyStatus = .checking
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

            Button(spotifyStatus.isConnected ? "Reconnect Spotify" : "Connect Spotify") {
                Task { await authorizeSpotify() }
            }
            .disabled(isAuthorizingSpotify)

            spotifyStatusView
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
        .task {
            await refreshSpotifyStatus()
        }
    }

    @ViewBuilder
    private var spotifyStatusView: some View {
        switch spotifyStatus {
        case .checking:
            Label("Checking connection status…", systemImage: "ellipsis.circle")
                .foregroundStyle(.secondary)
        case .connected(let username):
            Label("Spotify connected as \(username)", systemImage: "checkmark.circle")
                .foregroundStyle(.green)
        case .notConnected:
            EmptyView()
        }
    }

    private func refreshSpotifyStatus() async {
        do {
            if let username = try await session.spotifyStatus() {
                spotifyStatus = .connected(username: username)
            } else {
                spotifyStatus = .notConnected
            }
        } catch {
            spotifyStatus = .notConnected
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
            await refreshSpotifyStatus()
        } catch {
            spotifyStatus = .notConnected
            spotifyError = "Failed to connect Spotify: \(error)"
        }
    }
}

#Preview {
    SpotifyTab()
        .environment(SelectedESP32Store())
}


