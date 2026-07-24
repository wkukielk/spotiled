//
//  ManualTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI
import SpotiLEDKit

struct ManualTab: View {
    @State private var controller = ManualColorController()

    var body: some View {
        VStack() {
            Text("Manual mode")
                .navigationTitle("Manual mode")
            ColorPicker("Color", selection: $controller.color, supportsOpacity: false)
        }
        .padding(.horizontal)
    }
}

#Preview {
    ManualTab()
}

