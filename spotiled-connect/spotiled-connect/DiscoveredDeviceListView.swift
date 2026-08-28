//
//  DiscoveredControllerListView.swift
//  spotiled-connect
//
//  Created by wkuki on 10/08/2026.
//

import SwiftUI
import UIKit
import SpotiLEDKit

struct DiscoveredDeviceListView<RowContent: View>: View {
    @State private var browser: DeviceBrowser
    @Environment(\.openURL) private var openURL
    private let title: String
    private let emptyMessage: String
    private let rowContent: (DiscoveredDevice) -> RowContent

    init(
        browser: DeviceBrowser,
        title: String,
        emptyMessage: String,
        @ViewBuilder rowContent: @escaping (DiscoveredDevice) -> RowContent
    ) {
        _browser = State(initialValue: browser)
        self.title = title
        self.emptyMessage = emptyMessage
        self.rowContent = rowContent
    }

    var body: some View {
        Group {
            if browser.discoveredDevices.isEmpty {
                emptyStateView
            } else {
                List(browser.discoveredDevices) { controller in
                    rowContent(controller)
                }
            }
        }
        .navigationTitle(title)
        .task {
            browser.start()
        }
        .onDisappear {
            browser.stop()
        }
    }

    @ViewBuilder
    private var emptyStateView: some View {
        switch browser.state {
        case .searching:
            ContentUnavailableView(emptyMessage, systemImage: "wifi")
        case .localNetworkPermissionDenied:
            ContentUnavailableView {
                Label("Local Network Access Needed", systemImage: "wifi.exclamationmark")
            } description: {
                Text("Allow local network access for SpotiLED Connect in Settings to discover devices.")
            } actions: {
                Button("Open Settings") {
                    if let url = URL(string: UIApplication.openSettingsURLString) {
                        openURL(url)
                    }
                }
            }
        case .failed(let message):
            ContentUnavailableView {
                Label("Search Failed", systemImage: "exclamationmark.triangle")
            } description: {
                Text(message)
            } actions: {
                Button("Retry") {
                    browser.start()
                }
            }
        }
    }
}
