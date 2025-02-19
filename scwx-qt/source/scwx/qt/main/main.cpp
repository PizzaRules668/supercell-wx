#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <scwx/qt/config/county_database.hpp>
#include <scwx/qt/config/radar_site.hpp>
#include <scwx/qt/main/main_window.hpp>
#include <scwx/qt/main/versions.hpp>
#include <scwx/qt/manager/radar_product_manager.hpp>
#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/qt/manager/settings_manager.hpp>
#include <scwx/qt/manager/thread_manager.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/types/qt_types.hpp>
#include <scwx/qt/ui/setup/setup_wizard.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/util/environment.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/threads.hpp>

#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <boost/asio.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <QApplication>
#include <QStandardPaths>
#include <QTranslator>

static const std::string logPrefix_ = "scwx::main";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static void OverrideDefaultStyle(const std::vector<std::string>& args);
static void QtLogMessageHandler(QtMsgType                 messageType,
                                const QMessageLogContext& context,
                                const QString&            message);

int main(int argc, char* argv[])
{
   // Store arguments
   std::vector<std::string> args {};
   for (int i = 0; i < argc; ++i)
   {
      args.push_back(argv[i]);
   }

   // Initialize logger
   scwx::util::Logger::Initialize();
   spdlog::set_level(spdlog::level::debug);

   // Install Qt Message Handler
   qInstallMessageHandler(&QtLogMessageHandler);

   QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

   QApplication a(argc, argv);

   QCoreApplication::setApplicationName("Supercell Wx");
   scwx::network::cpr::SetUserAgent(
      fmt::format("SupercellWx/{}", scwx::qt::main::kVersionString_));

   // Enable internationalization support
   QTranslator translator;
   if (translator.load(QLocale(), "scwx", "_", ":/i18n"))
   {
      QCoreApplication::installTranslator(&translator);
   }

   if (!scwx::util::GetEnvironment("SCWX_TEST").empty())
   {
      QStandardPaths::setTestModeEnabled(true);
   }

   // Start the io_context main loop
   boost::asio::io_context& ioContext = scwx::util::io_context();
   auto                     work      = boost::asio::make_work_guard(ioContext);
   boost::asio::thread_pool threadPool {4};
   boost::asio::post(threadPool,
                     [&]()
                     {
                        while (true)
                        {
                           try
                           {
                              ioContext.run();
                              break; // run() exited normally
                           }
                           catch (std::exception& ex)
                           {
                              // Log exception and continue
                              logger_->error(ex.what());
                           }
                        }
                     });

   // Initialize AWS SDK
   Aws::SDKOptions awsSdkOptions;
   Aws::InitAPI(awsSdkOptions);

   // Initialize application
   scwx::qt::config::RadarSite::Initialize();
   scwx::qt::config::CountyDatabase::Initialize();
   scwx::qt::manager::SettingsManager::Instance().Initialize();
   scwx::qt::manager::ResourceManager::Initialize();

   // Theme
   auto uiStyle = scwx::qt::types::GetUiStyle(
      scwx::qt::settings::GeneralSettings::Instance().theme().GetValue());

   if (uiStyle == scwx::qt::types::UiStyle::Default)
   {
      OverrideDefaultStyle(args);
   }
   else
   {
      QApplication::setStyle(
         QString::fromStdString(scwx::qt::types::GetUiStyleName(uiStyle)));
   }

   // Run initial setup if required
   if (scwx::qt::ui::setup::SetupWizard::IsSetupRequired())
   {
      scwx::qt::ui::setup::SetupWizard w;
      w.show();
      a.exec();
   }

   // Run Qt main loop
   int result;
   {
      scwx::qt::main::MainWindow w;
      w.show();
      result = a.exec();
   }

   // Deinitialize application
   scwx::qt::manager::RadarProductManager::Cleanup();

   // Stop Qt Threads
   scwx::qt::manager::ThreadManager::Instance().StopThreads();

   // Gracefully stop the io_context main loop
   work.reset();
   threadPool.join();

   // Shutdown application
   scwx::qt::manager::ResourceManager::Shutdown();
   scwx::qt::manager::SettingsManager::Instance().Shutdown();

   // Shutdown AWS SDK
   Aws::ShutdownAPI(awsSdkOptions);

   return result;
}

static void
OverrideDefaultStyle([[maybe_unused]] const std::vector<std::string>& args)
{
#if defined(_WIN32)
   bool hasStyleArgument = false;

   for (int i = 1; i < args.size(); ++i)
   {
      if (args.at(i) == "-style")
      {
         hasStyleArgument = true;
         break;
      }
   }

   // Override the default Windows 11 style unless the user supplies a style
   // argument
   if (!hasStyleArgument)
   {
      QApplication::setStyle("windowsvista");
   }
#endif
}

void QtLogMessageHandler(QtMsgType                 messageType,
                         const QMessageLogContext& context,
                         const QString&            message)
{
   static const auto qtLogger_ = scwx::util::Logger::Create("qt");

   static const std::unordered_map<QtMsgType, spdlog::level::level_enum>
      levelMap_ {{QtMsgType::QtDebugMsg, spdlog::level::level_enum::debug},
                 {QtMsgType::QtInfoMsg, spdlog::level::level_enum::info},
                 {QtMsgType::QtWarningMsg, spdlog::level::level_enum::warn},
                 {QtMsgType::QtCriticalMsg, spdlog::level::level_enum::err},
                 {QtMsgType::QtFatalMsg, spdlog::level::level_enum::critical}};

   spdlog::level::level_enum level = spdlog::level::level_enum::info;
   auto                      it    = levelMap_.find(messageType);
   if (it != levelMap_.cend())
   {
      level = it->second;
   }

   spdlog::source_loc location {};
   if (context.file != nullptr && context.function != nullptr)
   {
      location = {context.file, context.line, context.function};
   }

   if (context.category != nullptr)
   {
      qtLogger_->log(
         location, level, "[{}] {}", context.category, message.toStdString());
   }
   else
   {
      qtLogger_->log(location, level, message.toStdString());
   }
}
