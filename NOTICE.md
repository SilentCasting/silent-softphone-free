# Silent Softphone Free — license notice · ліцензійне повідомлення

[English](#english) · [Українська](#українська)

## English

Copyright (C) 2026 Ihor Martseniuk and Silent Softphone Free contributors.

Silent Softphone Free is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version, together with the additional
terms below. See [LICENSE](LICENSE).

Silent Softphone Free is distributed in the hope that it will be useful, but **WITHOUT ANY
WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See sections 15 and 16 of the GNU General Public License.

### Additional terms (GPLv3 section 7)

These terms apply to the Silent Softphone Free code in this repository, as permitted by
section 7 of the GNU General Public License version 3:

1. **Notices** (section 7(b)). You must keep this NOTICE file, the copyright notices and the
   author attributions in all copies and modified versions.

2. **Modified versions** (section 7(c)). A modified version must be clearly marked as different
   from the original Silent Softphone Free: it must state that it was modified, by whom and when
   (see also section 5(a)), and it must not be presented as, or be confused with, the original
   Silent Softphone Free. Whoever distributes a modified version is solely responsible for it.

3. **Name and logo** (section 7(e)). This license does not grant any right to use the name
   "Silent Softphone Free" or its logo (the files in `assets/icon/`), except to truthfully
   describe the origin of the software. A modified version distributed to others must use a
   different name and logo.

### Third-party software

Silent Softphone Free binaries include or link the following software. Each component remains
under its own license.

| Component | Use | License |
|---|---|---|
| [PJSIP](https://www.pjsip.org) 2.17 (pjproject) | SIP/RTP stack, statically linked | GPL-2.0-or-later |
| [Qt](https://www.qt.io) 6 | UI framework, dynamically linked | LGPL-3.0-only |
| libsrtp 2.5 (bundled with PJSIP) | Secure RTP | BSD-3-Clause, Cisco Systems |
| Speex (bundled with PJSIP) | Codec, echo canceller, resampler | BSD-3-Clause, Xiph.Org Foundation and others |
| GSM 06.10 1.0.12 (bundled with PJSIP) | GSM codec | Permissive license, Jutta Degener and Carsten Bormann, TU Berlin |
| libresample 1.7 (bundled with PJSIP) | Sample-rate conversion | LGPL-2.1-or-later |
| WebRTC audio processing (bundled with PJSIP, Windows) | Echo cancellation | BSD-3-Clause, The WebRTC project authors |
| libyuv (bundled with PJSIP) | Built with PJSIP; not used (no video) | BSD-3-Clause, The LibYuv Project Authors |
| [Material Design icons](https://github.com/google/material-design-icons) | Icon shapes in `qml/Icon.qml` and the app icon | Apache-2.0, Google |

Excluded on purpose: the G.722.1 (Polycom) and iLBC (RFC 3951) codecs that ship with PJSIP,
because their licenses are unclear for redistribution.

### Commercial use of PJSIP

PJSIP is dual-licensed by Teluu Ltd. Using it under the GPL, as Silent Softphone Free does,
requires the whole program to be distributed under GPL-compatible terms with its source code.
Anyone who wants to build this code into a closed-source or otherwise non-GPL product needs a
commercial PJSIP license from Teluu (<https://www.pjsip.org/licensing.htm>), in addition to
complying with the licenses above. The authors of Silent Softphone Free do not provide such
licenses and are not responsible for how others use or modify this software.

---

## Українська

*Це переклад для зручності. Юридичну силу має англійський текст вище та [LICENSE](LICENSE).*

Copyright (C) 2026 Ihor Martseniuk та учасники проєкту Silent Softphone Free.

Silent Softphone Free — вільне програмне забезпечення: ви можете поширювати та/або змінювати
його на умовах GNU General Public License, опублікованої Free Software Foundation, версії 3
або (на ваш вибір) будь-якої пізнішої версії, разом із додатковими умовами нижче.
Див. [LICENSE](LICENSE).

Silent Softphone Free поширюється в надії, що буде корисним, але **БЕЗ БУДЬ-ЯКИХ ГАРАНТІЙ**,
навіть без неявної гарантії ПРИДАТНОСТІ ДО ПРОДАЖУ чи ВІДПОВІДНОСТІ ПЕВНІЙ МЕТІ. Див. розділи 15
і 16 GNU General Public License.

### Додаткові умови (розділ 7 GPLv3)

Ці умови стосуються коду Silent Softphone Free у цьому репозиторії, як це дозволяє розділ 7
GNU General Public License версії 3:

1. **Повідомлення** (розділ 7(b)). У всіх копіях і змінених версіях потрібно зберігати цей файл
   NOTICE, повідомлення про авторські права та зазначення авторства.

2. **Змінені версії** (розділ 7(c)). Змінена версія має бути чітко позначена як відмінна від
   оригінального Silent Softphone Free: у ній має бути вказано, що її змінено, ким і коли
   (див. також розділ 5(a)), і її не можна подавати як оригінальний Silent Softphone Free чи
   так, щоб їх можна було сплутати. Той, хто поширює змінену версію, самостійно відповідає за неї.

3. **Назва й логотип** (розділ 7(e)). Ця ліцензія не надає жодних прав на використання назви
   «Silent Softphone Free» чи її логотипа (файли в `assets/icon/`), окрім правдивого зазначення
   походження програми. Змінена версія, яку поширюють іншим, повинна мати іншу назву й логотип.

### Стороннє програмне забезпечення

Збірки Silent Softphone Free містять або підключають компоненти, перелічені в англійській
таблиці вище. Кожен компонент поширюється на умовах власної ліцензії.

Кодеки G.722.1 (Polycom) та iLBC (RFC 3951), що постачаються з PJSIP, свідомо виключено,
бо їхні ліцензії неоднозначні щодо поширення.

### Комерційне використання PJSIP

PJSIP має подвійне ліцензування від Teluu Ltd. Використання PJSIP на умовах GPL, як у Silent
Softphone Free, вимагає поширювати всю програму на GPL-сумісних умовах разом із вихідним кодом.
Той, хто хоче вбудувати цей код у закритий або інший не-GPL продукт, має придбати комерційну
ліцензію PJSIP у Teluu (<https://www.pjsip.org/licensing.htm>) і дотримуватися ліцензій вище.
Автори Silent Softphone Free не надають таких ліцензій і не відповідають за те, як інші
використовують або змінюють цю програму.
