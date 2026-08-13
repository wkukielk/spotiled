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

struct LabeledNumberField: View {
    let title: String
    @Binding var value: Int32
    var range: ClosedRange<Int32> = 0...Int32.max

    var body: some View {
        LabeledContent(title) {
            TextField(title, value: $value, format: .number)
                .keyboardType(.numberPad)
                .textFieldStyle(.roundedBorder)
                .multilineTextAlignment(.trailing)
                .onChange(of: value) { _, newValue in
                    value = min(max(newValue, range.lowerBound), range.upperBound)
                }
        }
    }
}
