// SPDX-FileCopyrightText: 2026 Ihor Martseniuk and Silent Softphone Free contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>

class Application;
class QQmlApplicationEngine;
class QQuickWindow;

// CI smoke test (--smoke-test): drives the main call flows against the mock backend
// as a sequence of steps, each waiting for the state it needs (registered, call
// connected, popup shown...) with a timeout, so slow CI machines can't skip checks.
// Optionally saves screenshots of key UI states (--screenshots <dir>).
// Fails on a timed-out step or on any QML warning.
class SmokeTest : public QObject
{
    Q_OBJECT

public:
    SmokeTest(Application &application, QQmlApplicationEngine &engine, const QString &screenshotDir);

    // Counts QML warnings; call before the QML engine loads anything.
    static void installMessageHandler();
    void start();

private:
    struct Step
    {
        QString name;
        std::function<bool()> ready;
        std::function<void()> action;
        int minimumWaitMs = 0;
    };

    void waitFor(const QString &name, std::function<bool()> ready, std::function<void()> action);
    void after(int delayMs, const QString &name, std::function<void()> action);
    void tick();
    void finish(bool success, const QString &message);

    bool callIn(int state) const;
    bool idle() const;
    bool popupVisible() const;

    QObject *root() const;
    QQuickWindow *popupWindow() const;
    void screenshot(const QString &name) const;
    void saveTrayIcons() const;
    void setComboPopupOpen(const char *objectName, bool open) const;
    void setRootProperty(const char *name, const QVariant &value) const;

    Application &m_application;
    QQmlApplicationEngine &m_engine;
    const QString m_screenshotDir;
    QList<Step> m_steps;
    qsizetype m_current = 0;
    QElapsedTimer m_stepClock;
    QTimer m_ticker;
};
