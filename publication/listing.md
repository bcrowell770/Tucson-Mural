# Tucson Mural â€” listing draft

Title: Tucson Mural
Author: BD Crowell
Type: Watchface
Release: 1.0.0
Short description: The time itself is a Tucson mural.

## Description (1097 characters)

The time itself is the mural.

Tucson Mural fills bold, readable numerals with original Sonoran Desert illustrations: Catalina Mountains, saguaros, prickly pear, wildflowers, desert wildlife, and architecture inspired by Tucson landmarks.

Morning light, bright desert skies, sunset colors, and a moonlit palette change both the background and the artwork inside the numbers throughout the day.

Features:
â€¢ Large illustrated numerals with strong outlines
â€¢ Local date, temperature, and weather conditions
â€¢ Fahrenheit or Celsius
â€¢ Optional Low Vision Mode for a larger date/weather line
â€¢ Optional Bluetooth disconnect icon and vibration
â€¢ Your watch's 12/24-hour preference
â€¢ Minute updates with no decorative animation

Supports Pebble Time 2, Pebble Round 2, Pebble Time, and Pebble Time Steel. Other models are not supported in this release.

Weather uses your phone's location and internet connection. Location permission is required for weather; the clock works without it. Weather data by Open-Meteo (https://open-meteo.com/), CC BY 4.0. Temperatures are rounded and converted for display.

## Assets

Each supported platform has five unframed native PNG screenshots in `store-assets/screenshots/`: morning, day, sunset, night, and Low Vision Mode. Use 02-day as the primary screenshot. Dimensions: Emery 200Ã—228, Gabbro 260Ã—260, Basalt 144Ã—168. Sample time/date/weather are controlled review fixtures, captured from the final live release in the SDK emulators.

`store-assets/` contains a 720Ã—320 marketing banner, 144Ã—144 and 80Ã—80 optional icons, and a larger editable-through-script banner master. These dimensions follow the existing Cozy Seasons packaging convention; final portal field requirements should be checked during upload. The banner uses framed screenshots; listing screenshots are unframed.

## Fields to review at submission

Choose the intended Pebble/Rebble developer account and confirm its public author/support contact. Support contact should use the selected developer account. Source code URL: https://github.com/bcrowell770/Tucson-Mural . No separate website has been selected. Public source visibility does not grant an additional redistribution license.

The current weather endpoint is Open-Meteo's free non-commercial API. This draft assumes a free, non-commercial watchface with no advertising. A paid/commercial release requires a suitable weather service arrangement before publication. No price has been set in any store.

## Publication references (checked 2026-10-10)

- Current Pebble developer dashboard: https://developer.repebble.com/dashboard
- Current Pebble store API/platform documentation: https://appstore-api.repebble.com/
- Rebble submission guidance: https://developer.rebble.io/guides/appstore-publishing/preparing-a-submission/
- Rebble screenshot/banner guidance: https://developer.rebble.io/guides/appstore-publishing/appstore-assets/
- Weather service terms: https://open-meteo.com/en/terms

The older Rebble guide specifies a 1,600-character description and up to five screenshots per platform. The current Pebble dashboard requires sign-in; authenticated upload fields were not inspected or changed.
