//
//  DiscoveredControllerListView.swift
//  spotiled-connect
//
//  Created by wkuki on 10/08/2026.
//

import SwiftUI
import UIKit
import SpotiLEDKit

struct DiscoveredDeviceListView<Scanner: DeviceScanning, RowContent: View>: View {
    @State private var scanner: Scanner
    @Environment(\.openURL) private var openURL
    private let title: String
    private let emptyMessage: String
    private let emptySystemImage: String
    private let rowContent: (Scanner.Device) -> RowContent

    init(
        scanner: Scanner,
        title: String,
        emptyMessage: String,
        emptySystemImage: String = "wifi",
        @ViewBuilder rowContent: @escaping (Scanner.Device) -> RowContent
    ) {
        _scanner = State(initialValue: scanner)
        self.title = title
        self.emptyMessage = emptyMessage
        self.emptySystemImage = emptySystemImage
        self.rowContent = rowContent
    }

    var body: some View {
        Group {
            if scanner.discoveredDevices.isEmpty {
                emptyStateView
            } else {
                List(scanner.discoveredDevices) { device in
                    rowContent(device)
                }
            }
        }
        .navigationTitle(title)
        .task {
            scanner.start()
        }
        .onDisappear {
            scanner.stop()
        }
    }

    @ViewBuilder
    private var emptyStateView: some View {
        switch scanner.discoveryPhase {
        case .searching:
            ContentUnavailableView(emptyMessage, systemImage: emptySystemImage)
        case .permissionDenied(let title, let message, let systemImage):
            ContentUnavailableView {
                Label(title, systemImage: systemImage)
            } description: {
                Text(message)
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
                    scanner.start()
                }
            }
        }
    }
}
