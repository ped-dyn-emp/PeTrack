#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <QApplication>
#include <QtTest>

int main( int argc, char* argv[] )
{
    QApplication a(argc, argv);

    QTEST_SET_MAIN_SOURCE_PATH
    const int result = Catch::Session().run( argc, argv );

    return ( result < 0xff ? result : 0xff );
}
