//
//  ManualTab.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI

struct ManualTab: View {
    @State private var color = Color(.mint)
    
    var body: some View {
        VStack() {
            Text("Provision device")
                .navigationTitle("Provision device")
            ColorPicker(/*@START_MENU_TOKEN@*/"Title"/*@END_MENU_TOKEN@*/, selection: $color, supportsOpacity: false)
        }
        .padding(.horizontal)
    }
}

#Preview {
    ManualTab()
}

