import SwiftUI

struct ContentView: View {
    var body: some View {
        VStack {
            Image(systemName: "thermometer.sun")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Too Hot To Ride")
                .font(.title)
                .fontWeight(.bold)
            Text("Check your Apple Watch for riding conditions!")
                .font(.subheadline)
                .foregroundColor(.secondary)
                .multilineTextAlignment(.center)
                .padding()
        }
        .padding()
    }
}

#Preview {
    ContentView()
}