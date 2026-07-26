//
//  Styles.swift
//  spotiled-connect
//
//  Created by wkuki on 23/07/2026.
//

import SwiftUI

struct PrimaryActionButtonStyle: PrimitiveButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        Button(role: configuration.role, action: configuration.trigger) {
            configuration.label
                .font(.title2)
                .padding(.vertical, 4)
                .frame(maxWidth: .infinity)
        }
        .buttonStyle(.borderedProminent)
    }
}

extension PrimitiveButtonStyle where Self == PrimaryActionButtonStyle {
    static var appPrimary: PrimaryActionButtonStyle { PrimaryActionButtonStyle() }
}

extension Color {
    static let cardBackground = Color(uiColor: .secondarySystemBackground)
    static let cardBorder = Color(uiColor: .separator)
}
