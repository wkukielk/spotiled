//
//  DiscoveredControllerListView.swift
//  spotiled-connect
//
//  Created by wkuki on 10/08/2026.
//

import SwiftUI
import SpotiLEDKit

struct DiscoveredDeviceListView<RowContent: View>: View {
    @State private var browser: DeviceBrowser
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
                ContentUnavailableView(emptyMessage, systemImage: "wifi")
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
}
