# Tucson Mural

Open `tucson-mural-1.0.0.pbw` with your Pebble phone app and select the watchface. This updates earlier Tucson Mural proofs using the same app identity.

Open the watchface settings in the phone app:

- Weather: choose Fahrenheit (default) or Celsius. Allow phone location access for local weather.
- Bluetooth: disconnect icon defaults on; disconnect vibration defaults off. Enable vibration for one short pulse when the phone disconnects. There are no repeated or startup alerts.
- Accessibility: enable Low Vision Mode for larger bold date/weather text. It defaults off. A long date may shorten to month/day to preserve room for the temperature.

Save settings to send them to the watch. The clock follows the watch's local time, date, and 12/24-hour preference. Four numerals are always shown, including a leading zero where needed.

Lighting follows fixed local hours: morning 06:00–09:59, day 10:00–16:59, sunset 17:00–19:59, night 20:00–05:59. These are artistic lighting periods, not calculated sunrise/sunset times.

Weather normally refreshes every 30 minutes. A dot beneath its icon means the observation is at least an hour old. After two hours without valid data, `--°` replaces the temperature. If weather is missing, check location permission, internet access, and the phone/watch connection. Weather uses the phone's location, wherever you are; it does not default to Tucson.

Supports Time 2, Round 2, Time, and Time Steel. The original Time Round and monochrome models need separate future ports.
