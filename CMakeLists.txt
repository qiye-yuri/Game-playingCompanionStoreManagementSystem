cmake_minimum_required(VERSION 3.19)
project(Game-playingCompanionStoreManagementSystem LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(Game-playingCompanionStoreManagementSystem
    WIN32 MACOSX_BUNDLE
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    LoginDlg.h LoginDlg.cpp LoginDlg.ui
    DatabaseManager.h
    DatabaseManager.cpp
    SecurityUtils.h
    SecurityUtils.cpp
    OrderManager.h
    OrderManager.cpp
    ValidatorUtils.h
    ValidatorUtils.cpp
    Logger.h
    ConfigManager.h
    Logger.cpp
    ForgotPasswordDlg.h
    ForgotPasswordDlg.cpp
    ForgotPasswordDlg.ui
    RegisterDlg.ui
    RegisterDlg.h
    RegisterDlg.cpp
    OrderSettlement.h
    OrderSettlement.cpp
    RechargeReviewDlg.h
    RechargeReviewDlg.cpp
    RechargeReviewDlg.ui
)

target_link_libraries(Game-playingCompanionStoreManagementSystem
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

include(GNUInstallDirs)

install(TARGETS Game-playingCompanionStoreManagementSystem
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Game-playingCompanionStoreManagementSystem
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
