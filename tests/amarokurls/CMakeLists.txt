include_directories(
        ${AMAROK_TEST_TREE}
        ${CMAKE_BINARY_DIR}/tests
        )

set( testamarokurls_SRCS TestAmarokUrls.cpp )
kde4_add_unit_test( testamarokurls ${testamarokurls_SRCS} )
target_link_libraries( testamarokurls amarokcore amaroklib ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

