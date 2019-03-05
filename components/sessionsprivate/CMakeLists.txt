set(sessionsprivateplugin_SRCS
    sessionsprivateplugin.cpp
)

add_library(sessionsprivateplugin SHARED ${sessionsprivateplugin_SRCS})
target_link_libraries(sessionsprivateplugin
    Qt5::Core
    Qt5::DBus
    Qt5::Quick
    Qt5::Qml
    Qt5::Gui
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::I18n
    PW::KWorkspace
)

install(TARGETS sessionsprivateplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/sessions)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/sessions)

