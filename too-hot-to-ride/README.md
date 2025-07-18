# Too Hot To Ride - Apple Watch Complication

An Apple Watch complication that helps cyclists and motorcyclists determine if conditions are too hot for safe riding based on a custom heat index formula.

## 🌡️ Heat Index Formula

The complication calculates a heat index using the formula:
```
Heat Index = Temperature (°F) + Humidity (%) - Wind Speed (mph)
```

## 🚦 Color-Coded Status

- **🟢 Green Circle** (< 130): Good to ride!
- **🟡 Yellow Circle** (130-169): Caution advised
- **🔴 Red Circle** (170+): Too hot to ride!

## 📱 Features

- **Apple Watch Complication**: Quick glance at riding conditions directly on your watch face
- **Real-time Weather Data**: Uses Apple's WeatherKit for accurate, location-based weather information
- **Multiple Complication Styles**: Supports Circular, Corner, and Inline accessory complications
- **Automatic Updates**: Timeline updates every hour with current and forecasted conditions
- **Location-based**: Automatically uses your current location for weather data

## 🏗️ Project Structure

```
TooHotToRide/
├── TooHotToRide.xcodeproj/          # Xcode project file
├── TooHotToRide/                    # iOS companion app
│   ├── TooHotToRideApp.swift       # iOS app entry point
│   ├── ContentView.swift           # iOS app main view
│   └── Assets.xcassets             # iOS app assets
├── TooHotToRide WatchKit App/       # watchOS app
│   ├── TooHotToRideApp.swift       # watchOS app entry point
│   ├── ContentView.swift           # watchOS app main view
│   ├── WeatherComplication.swift   # Weather manager and complication views
│   ├── ComplicationController.swift # Widget timeline provider
│   ├── Info.plist                  # watchOS app configuration
│   └── Assets.xcassets             # watchOS app assets
└── README.md                       # This file
```

## 🔧 Setup Requirements

### Prerequisites
- Xcode 15.0 or later
- iOS 17.0+ / watchOS 10.0+ deployment targets
- Apple Developer Account (for WeatherKit access)
- Physical Apple Watch (complications don't work in Simulator)

### WeatherKit Setup
1. **Enable WeatherKit**: In your Apple Developer account, ensure WeatherKit is enabled for your app's bundle identifier
2. **Add WeatherKit Capability**: In Xcode, go to your project settings → Signing & Capabilities → Add Capability → WeatherKit
3. **Update Bundle Identifier**: Change `com.toohottoride.app` to your own bundle identifier in the project settings

### Installation
1. Clone or download this repository
2. Open `TooHotToRide.xcodeproj` in Xcode
3. Update the bundle identifiers to match your Apple Developer account
4. Build and run on a physical iPhone with paired Apple Watch
5. Install the complication on your watch face

## 📊 How It Works

### Heat Index Calculation
1. **Temperature**: Converted from Celsius to Fahrenheit
2. **Humidity**: Converted from decimal (0.0-1.0) to percentage (0-100%)
3. **Wind Speed**: Converted from m/s to mph
4. **Formula Applied**: `Temp(°F) + Humidity(%) - WindSpeed(mph)`

### Example Calculations
- **Scenario 1**: 85°F, 60% humidity, 5 mph wind = 85 + 60 - 5 = **140** → 🟡 Yellow (Caution)
- **Scenario 2**: 75°F, 40% humidity, 10 mph wind = 75 + 40 - 10 = **105** → 🟢 Green (Good to ride)
- **Scenario 3**: 95°F, 80% humidity, 2 mph wind = 95 + 80 - 2 = **173** → 🔴 Red (Too hot)

### Complication Updates
- **Timeline**: Updates every hour with 4-hour forecast
- **Location**: Automatically detects current location
- **Fallback**: Shows error state if weather/location unavailable

## 🎯 Usage

### Adding to Watch Face
1. On your Apple Watch, press and hold the watch face
2. Tap "Edit" then swipe to customize complications
3. Tap a complication slot and select "Too Hot To Ride"
4. Press the Digital Crown to save

### Supported Complication Families
- **Accessory Circular**: Large circular complication with colored circle and icon
- **Accessory Corner**: Corner complication for corner slots
- **Accessory Inline**: Text-based inline complication

## 🔍 Troubleshooting

### Common Issues
1. **"Weather unavailable"**: Check internet connection and WeatherKit permissions
2. **"Location unavailable"**: Grant location permissions in Settings → Privacy & Security → Location Services
3. **Complication not updating**: Force close and reopen the Watch app, or restart Apple Watch

### Permissions Required
- **Location Services**: When In Use (for weather data)
- **WeatherKit**: Enabled in Apple Developer account

## 🛠️ Development

### Key Components
- **WeatherManager**: Handles location services and weather data fetching
- **WeatherComplicationProvider**: Manages widget timeline and data
- **WeatherComplicationEntry**: Data model for complication entries
- **WeatherComplicationEntryView**: SwiftUI view for complication display

### Customization
You can modify the heat index thresholds by editing the `getStatusInfo(for:)` function in `ComplicationController.swift`:

```swift
private func getStatusInfo(for heatIndex: Double) -> (Color, String, String) {
    if heatIndex < 130 {        // Change this threshold
        return (.green, "checkmark", "Good to ride!")
    } else if heatIndex < 170 { // Change this threshold
        return (.yellow, "exclamationmark", "Caution advised")
    } else {
        return (.red, "xmark", "Too hot to ride!")
    }
}
```

## 📄 License

This project is available under the MIT License. See the LICENSE file for more information.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## ⚠️ Disclaimer

This app provides general guidance based on weather conditions. Always use personal judgment and consider other factors when deciding whether conditions are safe for riding. The heat index calculation is a simplified formula and may not account for all environmental factors affecting comfort and safety.