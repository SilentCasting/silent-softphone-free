# Roadmap · План розвитку

[English](#english) · [Українська](#українська)

## English

### Goal

A light, native SIP softphone for Windows and macOS: a compact window, fast dialing, few clicks,
and a modern look with light and dark themes. Free and open source.

### Done (v0.1)

- SIP registration over UDP and TCP; multiple accounts with one active
- Outgoing and incoming calls, hold, mute, DTMF (RFC 2833 with SIP INFO fallback), key tones
- Incoming call popup on top of other windows
- Live audio level meters and the active codec during a call
- Codec list with priorities; G.722, G.711 A-law and µ-law enabled by default
- Contacts with search, call history
- Tray icon with status and missed calls, close to tray, start with the system
- Light, dark and system themes; adaptive layout
- SIP log file
- CI for Windows and macOS with an automated smoke test and screenshots

### Next

- TLS and SRTP (OpenSSL build of PJSIP)
- Audio device selection (microphone, speaker, ringer) and a ringtone
- Opus codec
- Passwords in Windows Credential Manager / macOS Keychain
- Contacts and history stored in SQLite
- Signed installers (MSI or NSIS for Windows, DMG for macOS)

### Later

- Call transfer (blind and attended), several simultaneous calls
- `tel:` / `sip:` / `callto:` links from the system, global hotkeys
- Auto-answer, do not disturb, call forwarding
- BLF / presence, instant messages (SIP MESSAGE)
- Conference calls, call recording, auto-provisioning
- Video calls, Linux

### Quality goals

- Cold start under 1 s, idle memory under 80 MB
- Installer under 40 MB
- Layout that works from a 320 px wide compact window to a wide window
- UI strings through `qsTr()`, ready for translations

## Українська

### Мета

Легкий нативний SIP softphone для Windows і macOS: компактне вікно, швидкий набір, мінімум
кліків і сучасний вигляд зі світлою та темною темами. Безкоштовний і з відкритим кодом.

### Зроблено (v0.1)

- Реєстрація SIP через UDP і TCP; кілька акаунтів, один активний
- Вихідні та вхідні дзвінки, hold, mute, DTMF (RFC 2833 з переходом на SIP INFO), тони клавіш
- Спливаюче вікно вхідного дзвінка поверх інших вікон
- Шкали рівня звуку й активний кодек під час розмови
- Список кодеків із пріоритетами; за замовчуванням увімкнені G.722, G.711 A-law і µ-law
- Контакти з пошуком, історія дзвінків
- Іконка в треї зі статусом і пропущеними, закриття в трей, автозапуск
- Світла, темна та системна теми; адаптивний макет
- SIP-журнал у файл
- CI для Windows і macOS з автоматичним smoke-тестом і скріншотами

### Далі

- TLS і SRTP (збірка PJSIP з OpenSSL)
- Вибір аудіопристроїв (мікрофон, динамік, дзвінок) і мелодія дзвінка
- Кодек Opus
- Паролі у Windows Credential Manager / macOS Keychain
- Контакти та історія в SQLite
- Підписані інсталятори (MSI або NSIS для Windows, DMG для macOS)

### Пізніше

- Переадресація дзвінка (blind і attended), кілька одночасних дзвінків
- Посилання `tel:` / `sip:` / `callto:` із системи, глобальні гарячі клавіші
- Автовідповідь, «не турбувати», переадресація дзвінків
- BLF / presence, повідомлення (SIP MESSAGE)
- Конференції, запис дзвінків, автопровізіонінг
- Відеодзвінки, Linux

### Цілі якості

- Холодний старт до 1 с, пам'ять у простої до 80 МБ
- Інсталятор до 40 МБ
- Макет, що працює від компактного вікна шириною 320 px до широкого
- Рядки інтерфейсу через `qsTr()`, готові до перекладу
