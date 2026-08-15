//
//  ContentView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct ContentView: View {
    @State private var selectedTab = 0

    var body: some View {
            TabView(selection: $selectedTab) {
                Tab("Spotify", systemImage: "wave.3.up.circle.fill", value: -1) {
                    SpotifyTab()
                }
                Tab("Home", systemImage: "house", value: 0) {
                    MiddleTab()
                }
                Tab("Manual", systemImage: "hand.tap.fill", value: 1) {
                    ManualTab()
                }
            }
    }
}

#Preview {
    ContentView()
        .environment(SelectedESP32Store())
}
