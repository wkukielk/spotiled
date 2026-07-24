//
//  SelectControllerView.swift
//  spotiled-connect
//
//  Created by wkuki on 19/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct SelectControllerView: View {
    @State private var browser = ControllerBrowser()

    var body: some View {
        List(browser.discoveredNames, id: \.self) { name in
            Text(name)
        }
        .overlay {
            if browser.discoveredNames.isEmpty {
                ContentUnavailableView("Searching for controllers…", systemImage: "wifi")
            }
        }
        .navigationTitle("Select controller")
        .task {
            browser.start()
        }
        .onDisappear {
            browser.stop()
        }
    }
}

#Preview {
    NavigationStack {
        SelectControllerView()
    }
}
