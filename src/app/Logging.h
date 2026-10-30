// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

namespace Logging {

// Mirrors all Qt log output, including PJSIP's SIP log, into
// <app data>/logs/silentsoftphone.log (the previous run is kept as .log.1).
// Returns the log file path, or an empty string if it couldn't be opened.
QString installFileLog();

} // namespace Logging
