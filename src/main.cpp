// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/Application.h"
#include "app/Logging.h"
#include "app/SmokeTest.h"
#include "models/CodecModel.h"

#ifdef SILENTSOFTPHONE_HAS_PJSIP
#include "sip/PjsipService.h"
#endif

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include <cstdio>
#include <memory>

namespace {

QString argumentValue(const QStringList &arguments, const QString &name)
{
    const qsizetype at = arguments.indexOf(name);
    return at >= 0 && at + 1 < arguments.size() ? arguments.at(at + 1) : QString();
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // QApplication: the tray icon and its menu are widgets
    // These names place the settings and logs: registry HKCU\Software\SilentSoftphone\SilentSoftphoneFree
    // on Windows, io.github.silentcasting.SilentSoftphoneFree.plist on macOS.
    QGuiApplication::setOrganizationName(QStringLiteral("SilentSoftphone"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("silentcasting.github.io"));
    QGuiApplication::setApplicationName(QStringLiteral("SilentSoftphoneFree"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Silent Softphone Free"));
    QGuiApplication::setApplicationVersion(QStringLiteral(SILENTSOFTPHONE_VERSION));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/icon/app.png")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    const QStringList arguments = app.arguments();

#ifdef SILENTSOFTPHONE_HAS_PJSIP
    // CI check that the PJSIP stack links, starts and opens its transports.
    if (arguments.contains(QStringLiteral("--sip-check"))) {
        PjsipService sip;
        const CodecModel codecDefaults(&sip, false); // applies the default codec preferences
        std::fprintf(stderr, "%s\n", qPrintable(sip.diagnostics()));
        std::fprintf(stderr, "codecs:");
        for (const SipCodec &codec : sip.codecs())
            std::fprintf(stderr, " %s(%d)", qPrintable(codec.id), codec.priority);
        std::fprintf(stderr, "\n");
        return sip.isReady() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif

    const bool smokeTest = arguments.contains(QStringLiteral("--smoke-test"));
    QString logFile;
    if (smokeTest)
        SmokeTest::installMessageHandler();
    else
        logFile = Logging::installFileLog();

    // The smoke test and --mock-sip run against the in-process PBX simulation.
    const bool useMock = smokeTest || arguments.contains(QStringLiteral("--mock-sip"));
    Application application(useMock ? Application::SipBackend::Mock : Application::defaultBackend(), logFile);
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    application.load(engine, arguments.contains(QStringLiteral("--minimized")));

    std::unique_ptr<SmokeTest> smoke;
    if (smokeTest) {
        smoke = std::make_unique<SmokeTest>(application, engine,
                                            argumentValue(arguments, QStringLiteral("--screenshots")));
        smoke->start();
    }

    return app.exec();
}
