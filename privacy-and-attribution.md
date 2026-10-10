# Privacy and attribution

Weather data by [Open-Meteo](https://open-meteo.com/), licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Temperature is rounded and optionally converted to Fahrenheit; weather codes are translated into original watch icons. Open-Meteo does not endorse this watchface.

The phone requests location when weather needs refreshing. Latitude/longitude are rounded to three decimals and sent over HTTPS to Open-Meteo. The weather cache stores temperature, condition, day/night state, and observation time; the watchface does not store coordinates in that cache. The operating system may supply a cached location up to 30 minutes old. Open-Meteo receives the request coordinates and IP address and describes its log retention in its own [privacy policy](https://open-meteo.com/en/terms#privacy).

Settings and weather observations are stored locally on the phone/watch. The watchface includes no developer-run server, advertising, or analytics. The normal Pebble phone app and weather provider have their own privacy policies.

Phone settings use @rebble/clay 1.0.10 under MIT; its notice is retained in the source archive under `licenses/Clay-MIT.txt`. The original mural imagery was developed with AI-assisted illustration and adapted to the supported Pebble displays; source artwork and prompt provenance are included in this repository. It is inspired by Tucson and the Sonoran Desert and is not a reproduction of an existing mural.

No open-source license has been selected for the original watchface code or artwork. Public visibility does not grant an additional redistribution license. Third-party components retain their stated licenses.
