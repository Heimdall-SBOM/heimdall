# Changelog

All notable changes to the Too Hot To Ride Apple Watch complication will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-07-18

### Added
- Initial release of Too Hot To Ride Apple Watch complication
- Heat index calculation using formula: Temperature (°F) + Humidity (%) - Wind Speed (mph)
- Color-coded status indicators:
  - 🟢 Green circle (< 130): Good to ride!
  - 🟡 Yellow circle (130-169): Caution advised
  - 🔴 Red circle (170+): Too hot to ride!
- Real-time weather data integration using Apple WeatherKit
- Location services for automatic weather detection
- Multiple complication families support:
  - Accessory Circular
  - Accessory Corner
  - Accessory Inline
- Automatic hourly timeline updates with 4-hour forecast
- iOS companion app with basic interface
- watchOS app with detailed riding conditions display
- Comprehensive error handling for weather/location unavailable scenarios

### Technical Features
- SwiftUI-based interface for both iOS and watchOS
- WeatherManager class for location services and weather data fetching
- Widget timeline provider for complication updates
- Proper unit conversions (Celsius to Fahrenheit, m/s to mph)
- Location permissions and WeatherKit integration
- Asset catalogs with app icons for both platforms