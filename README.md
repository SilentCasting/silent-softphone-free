<img src="assets/icon/app.png" width="96" alt="Silent Softphone Free">

# Silent Softphone Free

[English](#english) · [Українська](#українська)

## English

A lightweight, free and open-source SIP softphone for **Windows** and **macOS**, built with
Qt 6 and PJSIP.

> **Status:** early development, v0.1.0. Builds are not code-signed yet (see
> [Running the builds](#running-the-builds)).

### Features

- SIP over UDP and TCP (TLS is planned); several accounts, one active, switched from the
  header or the tray menu
- Calls: dial pad, hold, mute, DTMF for voice menus (RFC 2833 with automatic SIP INFO fallback;
  digits can be typed on the keyboard during a call), key tones
- Incoming call popup on top of other windows, even when the app is hidden in the tray
- Live microphone and speaker level meters and the active codec during a call
- Codec list: enable, disable and reorder (G.722 and G.711 A-law/µ-law by default)
- Contacts with search; call history with incoming, outgoing and missed calls
- Tray icon with registration status and missed calls; closing the window keeps the app
  running in the tray; optional start with the system
- Light, dark and system themes; compact and wide layouts
- SIP log file for troubleshooting

### Running the builds

Builds come from GitHub Actions: **Actions → latest "Build" run → Artifacts**
(`SilentSoftphoneFree-Windows`, `SilentSoftphoneFree-macOS`).

- **Windows:** unzip and run `SilentSoftphoneFree.exe`. If SmartScreen warns about an
  unrecognized app, choose **More info → Run anyway**.
- **macOS (Apple Silicon):** unzip, move **Silent Softphone Free.app** to Applications and open
  it. If macOS blocks it, open **System Settings → Privacy & Security** and press
  **Open Anyway**, or run
  `xattr -dr com.apple.quarantine "/Applications/Silent Softphone Free.app"` in Terminal.

### Building from source

Requirements: Qt 6.5+ (CI uses 6.10.1), CMake 3.21+, a C++17 compiler (MSVC 2022 on Windows,
Xcode on macOS); on Windows, Git Bash and Ninja for the PJSIP script.

```bash
tools/build_pjsip.sh "$PWD/pjsip"
cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt>/6.10.1/<arch> -DPj_DIR="$PWD/pjsip/lib/cmake/Pj"
cmake --build build --config Release
```

On Windows, run `build_pjsip.sh` from a Visual Studio developer environment (it builds with
Ninja and MSVC). Without `Pj_DIR` the app builds with the mock SIP backend only.

| Option | Effect |
|---|---|
| `--mock-sip` | Simulated PBX instead of PJSIP (sandbox: demo account, nothing saved) |
| `--minimized` | Start hidden in the tray (used by "start with the system") |
| `--sip-check` | Start PJSIP, print its version, transports and codecs, exit |
| `--smoke-test` | Run the call scenario against the mock backend and exit (CI) |
| `--screenshots <dir>` | With `--smoke-test`: save screenshots of key UI states |

**Logs:** Settings → Diagnostics → Open log folder. The log is `silentsoftphone.log`
(`%LOCALAPPDATA%\SilentSoftphone\SilentSoftphoneFree\logs` on Windows,
`~/Library/Application Support/SilentSoftphone/SilentSoftphoneFree/logs` on macOS);
the previous run is kept as `.log.1`.

### Project layout

```
src/
  app/           Application (composition root), Diagnostics, Logging, AutoStart, SmokeTest
  core/          Domain types and the SIP boundary: ISipService, MockSipService, CallSession
  sip/           PjsipService: ISipService on PJSIP (pjsua2)
  controllers/   PhoneController: call workflows for the UI
  models/        Contacts, call history, codecs
  settings/      Accounts, call settings, window settings
  ui/            ThemeManager, TrayController
qml/             Screens and components
assets/          App icon, macOS Info.plist, Windows resources
tools/           make_icons.py (icons), build_pjsip.sh (PJSIP build)
docs/            Architecture and roadmap
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and [docs/ROADMAP.md](docs/ROADMAP.md).

### License

**GPL-3.0-or-later** with additional terms under GPLv3 section 7 — see [LICENSE](LICENSE) and
[NOTICE.md](NOTICE.md).

- Free and open source, provided "as is", without warranty or liability of the authors
  (GPL sections 15–16).
- Anyone who modifies and distributes the code must mark their version as modified, give it a
  different name and logo, and is responsible for it.
- Third-party components (PJSIP under GPL-2.0-or-later, Qt under LGPL-3.0 and others) are listed
  in NOTICE.md.
- Building this code into a closed-source commercial product requires a commercial PJSIP
  license from Teluu.

---

## Українська

Легкий, безкоштовний SIP softphone з відкритим кодом для **Windows** і **macOS** на Qt 6 та PJSIP.

> **Статус:** рання розробка, v0.1.0. Збірки поки не підписані (див.
> [Запуск збірок](#запуск-збірок)).

### Можливості

- SIP через UDP і TCP (TLS у планах); кілька акаунтів, один активний, перемикання в шапці або
  в меню трею
- Дзвінки: номеронабирач, hold, mute, DTMF для голосових меню (RFC 2833 з автоматичним переходом
  на SIP INFO; цифри можна вводити з клавіатури під час розмови), тони клавіш
- Спливаюче вікно вхідного дзвінка поверх інших вікон, навіть коли застосунок у треї
- Шкали рівня мікрофона й динаміка та активний кодек під час розмови
- Список кодеків: увімкнення, вимкнення, порядок (за замовчуванням G.722 і G.711 A-law/µ-law)
- Контакти з пошуком; історія вхідних, вихідних і пропущених дзвінків
- Іконка в треї зі статусом реєстрації та пропущеними; закриття вікна залишає застосунок
  працювати в треї; автозапуск разом із системою
- Світла, темна та системна теми; компактний і широкий макети
- SIP-журнал у файл для діагностики

### Запуск збірок

Збірки беруться з GitHub Actions: **Actions → останній запуск «Build» → Artifacts**
(`SilentSoftphoneFree-Windows`, `SilentSoftphoneFree-macOS`).

- **Windows:** розпакуйте й запустіть `SilentSoftphoneFree.exe`. Якщо SmartScreen попереджає
  про невідомий застосунок, натисніть **Докладніше → Виконати в будь-якому разі**.
- **macOS (Apple Silicon):** розпакуйте, перенесіть **Silent Softphone Free.app** у «Програми»
  і відкрийте. Якщо macOS блокує запуск, відкрийте **Системні параметри → Приватність і безпека**
  і натисніть **Усе одно відкрити**, або виконайте в Терміналі
  `xattr -dr com.apple.quarantine "/Applications/Silent Softphone Free.app"`.

### Збірка з вихідного коду

Потрібно: Qt 6.5+ (CI використовує 6.10.1), CMake 3.21+, компілятор C++17 (MSVC 2022 на Windows,
Xcode на macOS); на Windows — Git Bash і Ninja для скрипта PJSIP.

```bash
tools/build_pjsip.sh "$PWD/pjsip"
cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt>/6.10.1/<arch> -DPj_DIR="$PWD/pjsip/lib/cmake/Pj"
cmake --build build --config Release
```

На Windows запускайте `build_pjsip.sh` із середовища розробника Visual Studio (збірка через
Ninja і MSVC). Без `Pj_DIR` застосунок збирається лише з mock-бекендом.

| Параметр | Дія |
|---|---|
| `--mock-sip` | Симуляція АТС замість PJSIP (пісочниця: демо-акаунт, нічого не зберігається) |
| `--minimized` | Запуск одразу в трей (так стартує автозапуск) |
| `--sip-check` | Запустити PJSIP, вивести версію, транспорти й кодеки, вийти |
| `--smoke-test` | Прогнати сценарій дзвінків на mock-бекенді й вийти (CI) |
| `--screenshots <dir>` | Разом із `--smoke-test`: зберегти скріншоти ключових станів UI |

**Журнал:** Settings → Diagnostics → Open log folder. Файл `silentsoftphone.log`
(`%LOCALAPPDATA%\SilentSoftphone\SilentSoftphoneFree\logs` на Windows,
`~/Library/Application Support/SilentSoftphone/SilentSoftphoneFree/logs` на macOS);
попередній запуск зберігається як `.log.1`.

### Структура проєкту

```
src/
  app/           Application (composition root), Diagnostics, Logging, AutoStart, SmokeTest
  core/          Доменні типи та межа SIP: ISipService, MockSipService, CallSession
  sip/           PjsipService: ISipService на PJSIP (pjsua2)
  controllers/   PhoneController: сценарії дзвінків для UI
  models/        Контакти, історія дзвінків, кодеки
  settings/      Акаунти, налаштування дзвінків і вікна
  ui/            ThemeManager, TrayController
qml/             Екрани та компоненти
assets/          Іконка, Info.plist для macOS, ресурси Windows
tools/           make_icons.py (іконки), build_pjsip.sh (збірка PJSIP)
docs/            Архітектура та план розвитку
```

Див. [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) і [docs/ROADMAP.md](docs/ROADMAP.md).

### Ліцензія

**GPL-3.0-or-later** з додатковими умовами за розділом 7 GPLv3 — див. [LICENSE](LICENSE) і
[NOTICE.md](NOTICE.md).

- Безкоштовний і відкритий; надається «як є», без гарантій і відповідальності авторів
  (розділи 15–16 GPL).
- Хто змінює й поширює код, зобов'язаний позначити свою версію як змінену, дати їй іншу назву
  й логотип і сам відповідає за неї.
- Сторонні компоненти (PJSIP під GPL-2.0-or-later, Qt під LGPL-3.0 та інші) перелічені в NOTICE.md.
- Для вбудовування цього коду в закритий комерційний продукт потрібна комерційна ліцензія PJSIP
  від Teluu.
