import SwiftUI
import CoreLocation

struct ContentView: View {
    @StateObject private var weatherManager = WeatherManager()
    
    var body: some View {
        VStack {
            // Status indicator circle
            Circle()
                .fill(weatherManager.statusColor)
                .frame(width: 60, height: 60)
                .overlay(
                    Image(systemName: weatherManager.statusIcon)
                        .foregroundColor(.white)
                        .font(.title2)
                )
            
            Text("Riding Conditions")
                .font(.headline)
                .padding(.top, 8)
            
            Text(weatherManager.statusText)
                .font(.caption)
                .foregroundColor(.secondary)
                .multilineTextAlignment(.center)
            
            if let score = weatherManager.currentScore {
                Text("Heat Index: \(Int(score))")
                    .font(.caption2)
                    .foregroundColor(.gray)
                    .padding(.top, 4)
            }
        }
        .padding()
        .onAppear {
            weatherManager.requestLocation()
        }
    }
}

#Preview {
    ContentView()
}