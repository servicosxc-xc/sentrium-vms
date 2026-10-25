#include <sentrium/platform.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

namespace {
QString qstring(std::string_view value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}
}

int main(int argc, char* argv[]) {
    using namespace sentrium;

    core::SystemClock clock;
    core::Lifecycle lifecycle(clock);
    std::string run = "unavailable";

    try {
        QGuiApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("SENTRIUM VMS"));
        app.setOrganizationName(QStringLiteral("SENTRIUM"));
        app.setApplicationVersion(qstring(core::version));

        run = platform_win::ascii(platform_win::run_id());

        auto root = platform_win::Directory::root();
        auto runtime = root.child(L"client-runtime");
        auto lock = runtime.file(L"sentrium-client.lock", true);
        auto logs = runtime.child(L"logs");
        platform_win::FileLog log(logs, core::Config{});

        const auto event = [&](std::string_view id, std::string_view message) {
            log.write(core::event_json(clock, "Info", id, "sentrium-client", message, run));
        };

        event("bootstrap-started", "Application bootstrap started");

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("sentriumProduct"), qstring(core::product));
        engine.rootContext()->setContextProperty(QStringLiteral("sentriumVersion"), qstring(core::version));
        engine.rootContext()->setContextProperty(QStringLiteral("sentriumBuildId"), qstring(core::build_id));
        engine.rootContext()->setContextProperty(QStringLiteral("sentriumBuildConfiguration"), qstring(core::build_configuration));
        event("ui-initializing", "Qt Quick interface initialization started");
        engine.loadFromModule("Sentrium.Client", "Main");

        if (engine.rootObjects().isEmpty()) {
            lifecycle.fail(core::Error::resource);
            event("ui-initialization-failed", "Qt Quick root object was not created");
            log.finish();
            return static_cast<int>(core::Error::resource);
        }

        lifecycle.ready();
        engine.rootObjects().front()->setProperty("runtimeState", qstring(core::name(lifecycle.health().state)));
        event("window-presented", "Main window created and runtime is ready");

        const int exit_code = app.exec();

        lifecycle.stopping();
        event("shutdown-started", "Application shutdown started");
        lifecycle.stopped();
        event("shutdown-completed", "Application shutdown completed");
        log.finish();

        return exit_code;
    } catch (const core::Failure& error) {
        lifecycle.fail(error.code());
        std::cerr << core::event_json(
            clock, "Error", "fatal", "sentrium-client", error.what(), run);
        return static_cast<int>(error.code());
    } catch (const std::exception& error) {
        lifecycle.fail(core::Error::internal);
        std::cerr << core::event_json(
            clock, "Error", "fatal", "sentrium-client", error.what(), run);
        return static_cast<int>(core::Error::internal);
    }
}
