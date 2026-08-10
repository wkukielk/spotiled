//
//  DiscoveredControllerListView.swift
//  spotiled-connect
//
//  Created by wkuki on 10/08/2026.
//

import SwiftUI
import SpotiLEDKit

/// Shared skeleton for browsing mDNS-discovered controllers: empty state while
/// searching, a list once found, and the browser's start/stop lifecycle. Callers
/// supply the discovery source, copy, and what a row looks like/does.
struct DiscoveredControllerListView<RowContent: View>: View {
    @State private var browser: ControllerBrowser
    private let title: String
    private let emptyMessage: String
    private let rowContent: (DiscoveredController) -> RowContent

    init(
        browser: ControllerBrowser,
        title: String,
        emptyMessage: String,
        @ViewBuilder rowContent: @escaping (DiscoveredController) -> RowContent
    ) {
        _browser = State(initialValue: browser)
        self.title = title
        self.emptyMessage = emptyMessage
        self.rowContent = rowContent
    }

    var body: some View {
        Group {
            if browser.discoveredControllers.isEmpty {
                ContentUnavailableView(emptyMessage, systemImage: "wifi")
            } else {
                List(browser.discoveredControllers) { controller in
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
