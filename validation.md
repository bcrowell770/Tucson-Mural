# Release validation â€” 2026-10-10

Version 1.0.0 changes release metadata only from the approved 0.11.0 build. Watch code, phone companion, and all artwork resources are byte-identical. UUID is unchanged.

Completed for this candidate:

- Clean dependency install from package-lock.json; pinned Clay 1.0.10.
- SDK 4.17 live build with POC_THEME / POC_HOUR / POC_MINUTE environment overrides explicitly removed.
- Host C policy tests and Node companion tests pass.
- Build-time silhouette validation passes all 4,320 times: 00:00â€“23:59 on each of the three platforms, including colon separation and round-screen edges.
- Final PBW installs on all three SDK emulators. Fifteen native screenshots demonstrate every numeral, all four lighting periods, and the larger footer.
- Resource packs: Emery 162,252 bytes (61.9% of 256 KiB); Gabbro 205,612 (78.4%); Basalt 88,396 (33.7%). Aggregate PBW size is not the per-watch resource usage.
- User has reported the time looks great on a physical watch and approved the revised larger footer. The exact hardware used was not stated.

Earlier 0.11.0 development checks covered Low Vision settings persistence/relaunch, long-date/extreme-temperature spacing, unchanged numeral pixels, weather age/expiry/failures, unit changes, and transition-only Bluetooth alerts. Emulator tests do not establish battery life or physical vibration feel.

Recommended before broad release: normal phone settings and location allow/deny on the intended phone, a real sustained disconnect/reconnect with vibration enabled, and ordinary wear to assess battery life. These results are not claimed complete. There is no measured multi-day battery result or guarantee.

The source and publication assets are prepared for GitHub. The watchface has not been published to an app store. No unsupported hardware is advertised as compatible.
