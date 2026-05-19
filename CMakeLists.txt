cmake_minimum_required(VERSION 3.16)

# ===== 强制指定编译器路径 =====
set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE PATH "" FORCE)
# ==============================

project(untitled1)

# 使用你实际安装的 Qt 路径
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64" CACHE PATH "Qt6 path")
set(Qt6_DIR "C:/Qt/6.10.2/mingw_64/lib/cmake/Qt6" CACHE PATH "Qt6 CMake directory")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 消除 Vulkan 警告
set(CMAKE_DISABLE_FIND_PACKAGE_Vulkan TRUE)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui

        # Common 模块
        src/common/databasemanager.cpp
        src/common/databasemanager.h
        src/common/destuil.cpp
        src/common/desutil.h
        src/common/hashsha.cpp
        src/common/hashsha.h
        src/common/user.cpp
        src/common/user.h
        src/common/passwordmanager.h
        ## sql
        src/common/sql/bill_sale.sql
        # Login 模块 (View & Controller)
        src/module/login/login_controllor.cpp
        src/module/login/login_controllor.h
        src/module/login/loginwidget.cpp
        src/module/login/loginwidget.h
        src/module/login/loginwidget.ui
        src/module/login/registerwidget.cpp
        src/module/login/registerwidget.h
        src/module/address/address_controllor.cpp
        src/module/address/address_controllor.h
        src/module/balance/balance_controllor.cpp
        src/module/balance/balance_controllor.h
        src/module/cart_in/cart_controllor.cpp
        src/module/cart_in/cart_controllor.h
        src/module/deduct/deduct_controllor.cpp
        src/module/deduct/deduct_controllor.h
        src/module/inventory/inventory_controllor.cpp
        src/module/inventory/inventory_controllor.h
        src/module/inventory/inventorywidget.cpp
        src/module/inventory/inventorywidget.h
        src/module/inventory/inventorywidget.ui
)

qt_add_executable(BillandSale_system
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
)

target_include_directories(BillandSale_system PRIVATE
        src
        src/common
        src/module/login
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(BillandSale_system PRIVATE
        Qt6::Widgets
        Qt6::Sql
)

if(WIN32 AND NOT CONAN_EXPORT_STAGE)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET BillandSale_system POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:BillandSale_system>\"
            COMMENT "Deploying Qt libraries..."
    )
endif()

set_target_properties(BillandSale_system PROPERTIES
        WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BillandSale_system)
endif()