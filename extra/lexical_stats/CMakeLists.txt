if(UDAV_BUILD_UNITTESTS)
  add_executable(ExtraLexicalStatsTestRunner)
  target_sources(ExtraLexicalStatsTestRunner
    PRIVATE
      FILE_SET CXX_MODULES
      FILES
        src/lexical_stats.cppm)
  target_link_libraries(ExtraLexicalStatsTestRunner
    PRIVATE
      UdavLib
      Catch2::Catch2WithMain)

  catch_discover_tests(ExtraLexicalStatsTestRunner
    TEST_PREFIX "UnitTest|")
endif()
